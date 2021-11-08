/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <bmetal/generated/autoconf.h>

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

char stack_main[CONFIG_MAIN_STACK_SIZE];
char stack_intr[CONFIG_NUM_CORES * CONFIG_INTR_STACK_SIZE];

void _prep_main(void)
{
}

void _prep_sub(void)
{
}
