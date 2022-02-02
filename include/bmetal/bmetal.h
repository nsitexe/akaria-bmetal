/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_BMETAL_H_
#define BAREMETAL_CRT_BMETAL_H_

#include <bmetal/app/bmetal/bmetal.h>

#define NMAX(a, b)     (((a) > (b)) ? (a) : (b))
#define NMIN(a, b)     (((a) < (b)) ? (a) : (b))
#define ARRAY_OF(x)    (sizeof(x) / sizeof((x)[0]))
#define ALIGN_OF(x, align)    (((x) % (align) == 0) ? (x) : ((x) / (align) + 1) * (align))

#define INT_TO_PTR(x)    ((void *)(x))
#define PTR_TO_INT(x)    ((intptr_t)(x))

#endif /* BAREMETAL_CRT_BMETAL_H_ */
