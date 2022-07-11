/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_BMETAL_H_
#define BAREMETAL_CRT_BMETAL_H_

#include <bmetal/app/bmetal/bmetal.h>

#ifdef __ASSEMBLER__

#define BIT(x)    (1 << x)

#else /* __ASSEMBLER__ */

#define NABS(a)        (((a) > 0) ? (a) : -(a))
#define NMAX(a, b)     (((a) > (b)) ? (a) : (b))
#define NMIN(a, b)     (((a) < (b)) ? (a) : (b))
#define ARRAY_OF(x)    (sizeof(x) / sizeof((x)[0]))
#define ALIGN_OF(x, align)    (((x) % (align) == 0) ? (x) : ((x) / (align) + 1) * (align))

#define INT_TO_PTR(x)    ((void *)(x))
#define PTR_TO_INT(x)    ((intptr_t)(x))

#define UPTR(x)    (uintptr_t)(x)

#define BIT(x)    (1UL << x)

#ifndef __aligned
#define __aligned(n)    __attribute__((aligned(n)))
#endif

#ifndef __section
#define __section(n)    __attribute__((section(n)))
#endif

#ifndef __used
#define __used          __attribute__((used))
#endif

#endif /* __ASSEMBLER__ */
#endif /* BAREMETAL_CRT_BMETAL_H_ */
