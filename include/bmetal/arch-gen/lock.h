/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_LOCK_H_
#define BAREMETAL_CRT_ARCH_GEN_LOCK_H_

#include <stdatomic.h>

#include <bmetal/bmetal.h>

#ifndef __arch_spinlock_t
typedef struct __arch_gen_spinlock {
	atomic_int val;
} __arch_gen_spinlock_t;
#define __arch_spinlock_t    __arch_gen_spinlock_t
#endif

#ifndef __arch_spinlock_init
#define __arch_spinlock_init    __arch_gen_spinlock_init
#endif

#ifndef __arch_spinlock_lock
#define __arch_spinlock_lock    __arch_gen_spinlock_lock
#endif

#ifndef __arch_spinlock_trylock
#define __arch_spinlock_trylock    __arch_gen_spinlock_trylock
#endif

#ifndef __arch_spinlock_unlock
#define __arch_spinlock_unlock    __arch_gen_spinlock_unlock
#endif

#endif /* BAREMETAL_CRT_ARCH_GEN_LOCK_H_ */
