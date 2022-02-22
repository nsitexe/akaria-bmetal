/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_INIT_H_
#define BAREMETAL_CRT_INIT_H_

#include <bmetal/bmetal.h>

#ifndef __aligned
#define __aligned(n)    __attribute__((aligned(n)))
#endif

#ifndef __section
#define __section(n)    __attribute__((section(n)))
#endif

#ifndef __used
#define __used          __attribute__((used))
#endif

#define define_stack(sym, size) \
	char __section(".noinit") sym[size]

#define define_init_func(fn) \
	const __init_func_t __init_func__##fn \
	__section(".initdata") __used = fn

#define define_fini_func(fn) \
	const __fini_func_t __fini_func__##fn \
	__section(".finidata") __used = fn

typedef int (*__init_func_t)(void);
typedef void (*__fini_func_t)(void);

extern char __stack_intr[];

#endif /* BAREMETAL_CRT_INIT_H_ */
