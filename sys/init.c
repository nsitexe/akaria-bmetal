/* SPDX-License-Identifier: Apache-2.0 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <bmetal/init.h>
#include <bmetal/arch.h>
#include <bmetal/comm.h>
#include <bmetal/intr.h>
#include <bmetal/libc_support.h>
#include <bmetal/lock.h>
#include <bmetal/printk.h>
#include <bmetal/string.h>
#include <bmetal/thread.h>
#include <bmetal/drivers/cpu.h>

#if (CONFIG_INTR_STACK_SIZE % CONFIG_STACK_ALIGN) != 0
#  error Invalid interrupt stack size. \
         Please check configs about INTR_STACK_SIZE and STACK_ALIGN.
#endif
#if (CONFIG_MAIN_CORE >= CONFIG_NUM_CORES) != 0
#  error Invalid main core. \
         Please check configs about MAIN_CORE and NUM_CORES.
#endif

extern char __bss_start[], __bss_end[];
extern char __sbss_start[], __sbss_end[];
extern char __data_start[], __data_end[], __data_load[];
extern char __sdata_start[], __sdata_end[], __sdata_load[];
extern char __tdata_start[], __tdata_end[], __tdata_load[];

extern __init_func_t __initcall_start[];
extern __init_func_t __initcall_end[];

static struct __aux_data __aux_start __section(BAREMETAL_CRT_AUX_SECTION) __aligned(8) __used;

define_stack(__stack_intr, CONFIG_NUM_CORES * CONFIG_INTR_STACK_SIZE);

/* +1 is for sentinel */
#define MAX_ARGV    (CONFIG_COMM_MAX_ARGS + 1 + CONFIG_COMM_MAX_ENVS + 1)
static char *argv[MAX_ARGV];
static char **envp;
static int index_argv = 0;

/* TODO: randomize */
static char at_random[16];

static const struct __comm_section __comm_s __section(BAREMETAL_CRT_COMM_SECTION) __aligned(8) __used = {
	.magic     = BAREMETAL_CRT_COMM_MAGIC,
};

static void clear_bss(void)
{
	kmemset(__bss_start, 0, __bss_end - __bss_start);
	kmemset(__sbss_start, 0, __sbss_end - __sbss_start);
}

static void copy_data(void)
{
#ifdef CONFIG_XIP
	kmemcpy(__data_start, __data_load, __data_end - __data_start);
	kmemcpy(__sdata_start, __sdata_load, __sdata_end - __sdata_start);
	kmemcpy(__tdata_start, __tdata_load, __tdata_end - __tdata_start);
#endif /* CONFIG_XIP */
}

static int init_proc_main(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __proc_info *pi = __proc_create();
	struct __thread_info *ti;
	int r;

	pi->pid = CONFIG_MAIN_PID;

	/* Init stdin/out/err */
	r = __file_stdio_init(pi);
	if (r) {
		return r;
	}

	/* Init thread info */
	ti = __thread_create(pi);
	if (!ti) {
		return -EAGAIN;
	}

	r = __thread_run(ti, cpu);
	if (r) {
		return r;
	}

	return 0;
}

static int init_proc_sub(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __proc_info *pi = __proc_get_current();
	struct __thread_info *ti;
	int r;

	/* Init thread info */
	ti = __thread_create(pi);
	if (!ti) {
		return -EAGAIN;
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
	int res = 0;

	for (int i = 0; i < cnt; i++) {
		int r;

		r = __initcall_start[i]();
		if (r) {
			printk("Initcall failed.\n");
			res = r;
		}
	}

	return res;
}

static int add_argv(void *p)
{
	if (index_argv >= MAX_ARGV) {
		printk("Exceed number of argv max:%d.\n", MAX_ARGV);
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
		printk("Exceed number of auxv max:%d.\n", MAX_ARGV);
		return -ENOMEM;
	}

	argv[index_argv] = (void *)(intptr_t)typ;
	argv[index_argv + 1] = p;
	index_argv += 2;

	return 0;
}

static int load_argv(const struct __comm_area_header *h, const char *buf_args)
{
	const struct __comm_arg_header *ha;
	uintptr_t buf = (uintptr_t)buf_args;
	int r;

	for (int i = 0; i < h->num_args; i++) {
		ha = (const struct __comm_arg_header *)buf;
		buf += sizeof(struct __comm_arg_header);

		r = add_argv((void *)buf);
		if (r) {
			printk("Too many argumnets, max:%d.\n", MAX_ARGV);
		}
		buf += ha->size;

		buf = ALIGN_OF(buf, 8);
	}

	return 0;
}

static int init_args(int *argc)
{
	struct __comm_area_header *h_area = (struct __comm_area_header *)__comm_area;

	index_argv = 0;

	if (h_area->magic == BAREMETAL_CRT_COMM_MAGIC) {
		size_t sz = ALIGN_OF(sizeof(struct __comm_area_header), 8);

		if (CONFIG_COMM_MAX_ARGS < h_area->num_args) {
			printk("Exceed number of args (req:%" PRId32 ", max:%d)\n",
				h_area->num_args, CONFIG_COMM_MAX_ARGS);
		}

		load_argv(h_area, __comm_area + sz);
		if (index_argv != h_area->num_args) {
			printk("Illegal number of arguments (ind:%d != num_args:%d).\n",
				index_argv, h_area->num_args);
		}
	}
	if (argv[0] == NULL) {
		printk("Missing kernel name. Use default '%s'.\n", DEFAULT_KERNEL_NAME);
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

	if (__aux_start.valid) {
		add_aux(AT_PHENT, (void *)__aux_start.phent);
		add_aux(AT_PHNUM, (void *)__aux_start.phnum);

		char *p = (char *)&__aux_start;
		p += sizeof(__aux_start);

		add_aux(AT_PHDR, p);
	} else {
		printk("Missing program header. AT_PHDR is not available.\n");
	}

	add_aux(0, NULL);

	return 0;
}

void __prep_main(void)
{
	int r;

	clear_bss();
	copy_data();

	init_drivers();
	init_proc_main();

	/* Boot other cores */
	__cpu_wakeup_all();

	r = __cpu_on_wakeup(__cpu_get_current());
	if (r) {
		printk("prep_main: failed to callback on_wakeup.\n");
	}

	/* FIXME: tentative */
	printk("hello %d\n", __thread_get_tid());
	__intr_enable_local();

	int argc;
	init_args(&argc);

	__libc_init(argc, argv, envp);
}

void __prep_sub(void)
{
	int r;

	init_proc_sub();

	r = __cpu_on_wakeup(__cpu_get_current());
	if (r) {
		printk("prep_sub: failed to callback on_wakeup.\n");
	}

	/* FIXME: tentative */
	printk("hello %d\n", __thread_get_tid());
	__intr_enable_local();
}
