/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_SYNC_H_
#define BAREMETAL_CRT_ARCH_GEN_SYNC_H_

#include <bmetal/bmetal.h>

#ifndef imb
/* Instruction memory barrier */
#define imb()
#endif

#ifndef dmb
/* Data memory barrier */
#define dmb()
#endif

#ifndef drmb
/* Data memory barrier for read */
#define drmb() dmb()
#endif

#ifndef dwmb
/* Data memory barrier for write */
#define dwmb() dmb()
#endif

#ifndef noop
/* No operation */
#define noop()
#endif

#endif /* BAREMETAL_CRT_ARCH_GEN_SYNC_H_ */
