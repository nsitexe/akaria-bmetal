/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_FUTEX_H_
#define BAREMETAL_CRT_SYS_FUTEX_H_

#include <bmetal/bmetal.h>

#define FUTEX_WAIT               0
#define FUTEX_WAKE               1
#define FUTEX_REQUEUE            3
#define FUTEX_CMP_REQUEUE        4
#define FUTEX_LOCK_PI		 6
#define FUTEX_UNLOCK_PI		 7
#define FUTEX_TRYLOCK_PI	 8
#define FUTEX_WAIT_BITSET	 9
#define FUTEX_WAKE_BITSET	 10
#define FUTEX_WAIT_REQUEUE_PI    11
#define FUTEX_CMP_REQUEUE_PI     12
#define FUTEX_LOCK_PI2		 13

#define FUTEX_PRIVATE_FLAG	 128
#define FUTEX_CLOCK_REALTIME	 256
#define FUTEX_MASK               ~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)

#define FUTEX_BITSET_ANY         0xffffffff

#endif /* BAREMETAL_CRT_SYS_FUTEX_H_ */
