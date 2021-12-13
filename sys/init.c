/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <bmetal/init.h>
#include <bmetal/arch.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/drivers/cpu.h>

#if (CONFIG_MAIN_STACK_SIZE % CONFIG_STACK_ALIGN) != 0
#  error Invalid main stack size. \
         Please check configs about MAIN_STACK_SIZE and STACK_ALIGN.
#endif
#if (CONFIG_INTR_STACK_SIZE % CONFIG_STACK_ALIGN) != 0
#  error Invalid interrupt stack size. \
         Please check configs about INTR_STACK_SIZE and STACK_ALIGN.
#endif
#if (CONFIG_MAIN_CORE >= CONFIG_NUM_CORES) != 0
#  error Invalid main core. \
         Please check configs about MAIN_CORE and NUM_CORES.
#endif

extern char __bss_start[], __bss_end[], __bss_load[];
extern char __sbss_start[], __sbss_end[], __sbss_load[];
extern char __data_start[], __data_end[], __data_load[];
extern char __sdata_start[], __sdata_end[], __sdata_load[];

extern __init_func_t __initcall_start[];
extern __init_func_t __initcall_end[];

char __stack_main[CONFIG_MAIN_STACK_SIZE];
char __stack_intr[CONFIG_NUM_CORES * CONFIG_INTR_STACK_SIZE];

void __libc_init(void);
int main(int argc, char *argv[], char *envp[]);

static void clear_bss(void)
{
	memset(__bss_load, 0, __bss_end - __bss_start);
	memset(__sbss_load, 0, __sbss_end - __sbss_start);
}

static void copy_data(void)
{
	memcpy(__data_start, __data_load, __data_end - __data_start);
	memcpy(__sdata_start, __sdata_load, __sdata_end - __sdata_start);
}

void __prep_main(void)
{
	clear_bss();

#ifdef CONFIG_XIP
	copy_data();
#endif /* CONFIG_XIP */

	/* Init drivers */
	int cnt = __initcall_end - __initcall_start;
	for (int i = 0; i < cnt; i++) {
		int r;

		r = __initcall_start[i]();
		if (r) {
			printk("Initcall failed.\n");
		}
	}

	/* Init libc */
	__libc_init();

	/* Init process info */
	struct __process_info *pi = __get_current_process();

	pi->pid = CONFIG_MAIN_PID;
	pi->fdset[0] = 0;
	pi->fdset[1] = 1;
	pi->fdset[2] = 2;

	/* Init thread info */
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		struct __thread_info *ti = __get_raw_thread(i);

		ti->tid = i + CONFIG_MAIN_PID;
	}

	/* Set thread pointer */
	__set_current_thread(__get_raw_thread(0));

	/* Boot other cores */
	__cpu_wakeup_all();

	char *argv[1];
	char *envp[2];

	argv[0] = "app";

	envp[0] = NULL;
	envp[1] = NULL;

	main(1, argv, envp);
}

void __prep_sub(void)
{
}
