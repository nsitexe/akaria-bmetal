/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/init.h>
#include <bmetal/arch.h>
#include <bmetal/comm.h>
#include <bmetal/fini.h>
#include <bmetal/intr.h>
#include <bmetal/libc_support.h>
#include <bmetal/lock.h>
#include <bmetal/printk.h>
#include <bmetal/syscall.h>
#include <bmetal/thread.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/sys/elf.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>
#include <bmetal/sys/string.h>

/* +1 is for sentinel, +10 for auxiliary vectors */
#define MAX_ARGV    (CONFIG_COMM_MAX_ARGS + 1 + CONFIG_COMM_MAX_ENVS + 1 + 10)

#if (CONFIG_INTR_STACK_SIZE % CONFIG_STACK_ALIGN) != 0
#  error Invalid interrupt stack size. \
         Please check configs about INTR_STACK_SIZE and STACK_ALIGN.
#endif
#if (CONFIG_IDLE_STACK_SIZE % CONFIG_STACK_ALIGN) != 0
#  error Invalid idle stack size. \
         Please check configs about IDLE_STACK_SIZE and STACK_ALIGN.
#endif
#if (CONFIG_MAIN_STACK_SIZE % CONFIG_STACK_ALIGN) != 0
#  error Invalid main stack size. \
         Please check configs about MAIN_STACK_SIZE and STACK_ALIGN.
#endif
#if CONFIG_MAIN_STACK_SIZE < 8 * MAX_ARGV
#  error Too small main stack size. Cannot put argv on stack. \
         Please check configs about MAIN_STACK_SIZE.
#endif

extern char __bss_start[], __bss_end[];
extern char __sbss_start[], __sbss_end[];
extern char __preinit_array_start[], __preinit_array_end[], __preinit_array_load[];
extern char __init_array_start[], __init_array_end[], __init_array_load[];
extern char __fini_array_start[], __fini_array_end[], __fini_array_load[];
extern char __data_start[], __data_end[], __data_load[];
extern char __sdata_start[], __sdata_end[], __sdata_load[];
extern char __tdata_start[], __tdata_end[], __tdata_load[];
#ifdef CONFIG_64BIT
extern const Elf64_Ehdr __ehdr_start;
#else
extern const Elf32_Ehdr __ehdr_start;
#endif /* CONFIG_64BIT */

extern __init_func_t __initcall_start[];
extern __init_func_t __initcall_end[];

static const struct __aux_data __aux_start__ __section(BAREMETAL_CRT_AUX_SECTION) __aligned(8) __used;
static volatile const struct __aux_data *__aux_start = &__aux_start__;

define_stack(__stack_intr, CONFIG_NUM_CORES * CONFIG_INTR_STACK_SIZE);
define_stack(__stack_idle, CONFIG_NUM_CORES * CONFIG_IDLE_STACK_SIZE);
define_stack(__stack_main, CONFIG_MAIN_STACK_SIZE);

static char **argv;
static char **envp;
static int index_argv;

/* TODO: randomize */
static char at_random[16];

static const struct __comm_section __comm_s __section(BAREMETAL_CRT_COMM_SECTION) __aligned(8) __used = {
	.magic     = BAREMETAL_CRT_COMM_MAGIC,
};

static void clear_bss(void)
{
#ifdef CONFIG_CLEAR_BSS
	kmemset(__bss_start, 0, __bss_end - __bss_start);
	kmemset(__sbss_start, 0, __sbss_end - __sbss_start);
#endif /* CONFIG_CLEAR_BSS */
}

static void copy_data(void)
{
#ifdef CONFIG_XIP
	kmemcpy(__preinit_array_start, __preinit_array_load, __preinit_array_end - __preinit_array_start);
	kmemcpy(__init_array_start, __init_array_load, __init_array_end - __init_array_start);
	kmemcpy(__fini_array_start, __fini_array_load, __fini_array_end - __fini_array_start);
	kmemcpy(__data_start, __data_load, __data_end - __data_start);
	kmemcpy(__sdata_start, __sdata_load, __sdata_end - __sdata_start);
	kmemcpy(__tdata_start, __tdata_load, __tdata_end - __tdata_start);
#endif /* CONFIG_XIP */
}

static int init_proc(void)
{
	struct __proc_info *pi = __proc_create();
	int r;

	pi->pid = CONFIG_MAIN_PID;

	/* Init stdin/out/err */
	r = __file_stdio_init(pi);
	if (r) {
		return r;
	}

	return 0;
}

static int fini_proc(void)
{
	return 0;
}

static int init_idle_thread(struct __cpu_device *cpu, int leader)
{
	struct __proc_info *pi = __proc_get_current();
	struct __thread_info *ti;
	size_t pos_idle = (cpu->id_cpu + 1) * CONFIG_IDLE_STACK_SIZE;
	size_t pos_intr = (cpu->id_cpu + 1) * CONFIG_INTR_STACK_SIZE;
	char *sp_user = &__stack_idle[pos_idle];
	char *sp_intr = &__stack_intr[pos_intr];

	ti = __thread_create(pi);
	if (!ti) {
		return -EAGAIN;
	}

	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_1, leader);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK, (uintptr_t)sp_user);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK_INTR, (uintptr_t)sp_intr);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_INTADDR, (uintptr_t)__thread_idle_main);

	__cpu_set_thread_idle(cpu, ti);

	return 0;
}

static int init_main_thread(int argc, char *argv[], char *envp[], char *sp_user, char *sp_intr)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __proc_info *pi = __proc_get_current();
	struct __thread_info *ti;
	int r;

	ti = __thread_create(pi);
	if (!ti) {
		return -EAGAIN;
	}

	__init_main_thread_args(ti, argc, argv, envp, sp_user, sp_intr);

	r = __proc_set_leader(pi, ti);
	if (r) {
		return r;
	}

	r = __thread_run(ti, cpu);
	if (r) {
		return r;
	}

	return 0;
}

static int init_drivers(void)
{
	int cnt = __initcall_end - __initcall_start;
	int r, res = 0;

	__device_set_probe_all_enabled(0);

	for (int i = 0; i < cnt; i++) {
		r = __initcall_start[i]();
		if (r) {
			pri_err("Initcall failed.\n");
			res = r;
		}
	}

	__device_set_probe_all_enabled(1);
	do {
		r = __device_probe_all();
		if (IS_ERROR(r)) {
			res = r;
			break;
		}
	} while (r == -EAGAIN);

	return res;
}

static int fini_drivers(void)
{
	return 0;
}

static int add_argv(void *p)
{
	if (index_argv >= MAX_ARGV) {
		pri_warn("Exceed number of argv max:%d.\n", MAX_ARGV);
		return -ENOMEM;
	}

	argv[index_argv] = p;
	index_argv++;

	return 0;
}

static int add_env(void *p)
{
	return add_argv(p);
}

static int add_aux(int typ, void *p)
{
	if (index_argv >= MAX_ARGV - 1) {
		pri_warn("Exceed number of auxv max:%d.\n", MAX_ARGV);
		return -ENOMEM;
	}

	argv[index_argv] = (void *)(intptr_t)typ;
	argv[index_argv + 1] = p;
	index_argv += 2;

	return 0;
}

static int map_argv(const struct __comm_area_header *h, const char *buf_args)
{
	const struct __comm_arg_header *ha;
	uintptr_t buf = (uintptr_t)buf_args;
	int r;

	for (int i = 0; i < h->num_args; i++) {
		struct __cpu_device *cpu = __cpu_get_current();
		ha = (const struct __comm_arg_header *)buf;
		buf += sizeof(struct __comm_arg_header);

		r = __cpu_cache_inv_range(cpu, (void *)buf, ha->size);
		if (r) {
			pri_warn("invalidate arg:%d is failed, arguments may be broken.\n", i);
		}

		r = add_argv((void *)buf);
		if (r) {
			pri_warn("Too many argumnets, max:%d.\n", MAX_ARGV);
		}
		buf += ha->size;

		buf = ALIGN_OF(buf, 8);
	}

	return 0;
}

static int unmap_argv(const struct __comm_area_header *h, const char *buf_args)
{
	const struct __comm_arg_header *ha;
	uintptr_t buf = (uintptr_t)buf_args;
	int r;

	for (int i = 0; i < h->num_args; i++) {
		struct __cpu_device *cpu = __cpu_get_current();
		ha = (const struct __comm_arg_header *)buf;
		buf += sizeof(struct __comm_arg_header);

		r = __cpu_cache_flush_range(cpu, (void *)buf, ha->size);
		if (r) {
			pri_warn("flush arg:%d is failed, arguments may be broken.\n", i);
		}

		buf += ha->size;

		buf = ALIGN_OF(buf, 8);
	}

	return 0;
}

static int init_args(int *argc)
{
	struct __comm_area_header *h_area = (struct __comm_area_header *)__comm_area;

	/* argv, envp and auxv are constructed on stack of main thread */
	argv = (void *)&__stack_main[CONFIG_MAIN_STACK_SIZE];
	argv -= MAX_ARGV;

	index_argv = 0;

	if (h_area->magic == BAREMETAL_CRT_COMM_MAGIC) {
		size_t sz = ALIGN_OF(sizeof(struct __comm_area_header), 8);

		if (CONFIG_COMM_MAX_ARGS < h_area->num_args) {
			pri_warn("Exceed number of args (req:%" PRId32 ", max:%d)\n",
				h_area->num_args, CONFIG_COMM_MAX_ARGS);
		}

		map_argv(h_area, __comm_area + sz);
		if (index_argv != h_area->num_args) {
			pri_warn("Illegal number of arguments (ind:%d != num_args:%" PRId32 ").\n",
				index_argv, h_area->num_args);
		}
	}
	if (index_argv == 0) {
		pri_info("Missing kernel name. Use default '%s'.\n", DEFAULT_KERNEL_NAME);
		argv[0] = DEFAULT_KERNEL_NAME;
		index_argv = 1;
	}

	*argc = index_argv;
	index_argv++;

	/* Environment variables */
	envp = &argv[index_argv];

	/* No environment variables */
	add_env(NULL);

	/* Auxiliary vectors */
	add_aux(AT_RANDOM, at_random);
	add_aux(AT_PAGESZ, (void *)__PAGE_SIZE);

	if (__aux_start->valid) {
		add_aux(AT_PHENT, (void *)(uintptr_t)__aux_start->phent);
		add_aux(AT_PHNUM, (void *)(intptr_t)__aux_start->phnum);

		char *p = (char *)&__ehdr_start;
		p += __ehdr_start.e_phoff;

		add_aux(AT_PHDR, p);
	} else {
		pri_warn("Missing program header. AT_PHDR is not available.\n");
	}

	add_aux(0, NULL);

	return 0;
}

static int fini_args(int st)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __comm_area_header *h_area = (struct __comm_area_header *)__comm_area;

	if (h_area->magic == BAREMETAL_CRT_COMM_MAGIC) {
		size_t sz = ALIGN_OF(sizeof(struct __comm_area_header), 8);

		if (CONFIG_COMM_MAX_ARGS < h_area->num_args) {
			pri_warn("Broken number of args (req:%" PRId32 ", max:%d)\n",
				h_area->num_args, CONFIG_COMM_MAX_ARGS);
		}

		unmap_argv(h_area, __comm_area + sz);

		h_area->ret_main = st;
		h_area->done = 1;
		__cpu_cache_flush_range(cpu, &h_area->ret_main, sizeof(h_area->ret_main));
		__cpu_cache_flush_range(cpu, &h_area->done, sizeof(h_area->done));
	}

	return 0;
}

void __init_system(void)
{
	clear_bss();
	copy_data();

	init_drivers();
	init_proc();
}

void __fini_system(void)
{
#ifdef CONFIG_PWR_OFF_AFTER_EXIT
	__fini_power_off();
#endif /* CONFIG_PWR_OFF_AFTER_EXIT */

	fini_proc();
	fini_drivers();
}

void __init_leader(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	int argc;

	pri_info("hello cpu:%d phys:%d\n", cpu->id_cpu, cpu->id_phys);

	/* Boot other cores */
	__cpu_wakeup_all();

	init_args(&argc);
	init_main_thread(argc, argv, envp, &__stack_main[CONFIG_MAIN_STACK_SIZE], &__stack_intr[CONFIG_INTR_STACK_SIZE]);
	init_idle_thread(cpu, 1);

	dwmb();

	/* Switch to idle/main thread */
	__arch_context_switch();
}

void __fini_leader(int status)
{
	int st = status & 0xff;

	fini_args(st);

	/* Stop other cores */
	__cpu_sleep_all();
}

void __init_child(void)
{
	struct __cpu_device *cpu = __cpu_get_current();

	pri_info("hello cpu:%d phys:%d\n", cpu->id_cpu, cpu->id_phys);

	init_idle_thread(cpu, 0);

	dwmb();

	/* Switch to idle/main thread */
	__arch_context_switch();
}

void __fini_child(int status)
{
}
