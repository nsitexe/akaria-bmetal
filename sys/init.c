/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <bmetal/bmetal.h>

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

extern char __bss_start[];
extern char __bss_end[];
extern char __bss_load[];
extern char __data_start[];
extern char __data_end[];
extern char __data_load[];

char stack_main[CONFIG_MAIN_STACK_SIZE];
char stack_intr[CONFIG_NUM_CORES * CONFIG_INTR_STACK_SIZE];

void __libc_init(void);
int main(int argc, char *argv[], char *envp[]);

static void __clear_bss(void)
{
	memset(__bss_load, 0, __bss_end - __bss_start);
}

static void __copy_data(void)
{
	memcpy(__data_start, __data_load, __data_end - __data_start);
}

void __prep_main(void)
{
	__clear_bss();

#ifdef CONFIG_XIP
	__copy_data();
#endif /* CONFIG_XIP */

	__libc_init();

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
