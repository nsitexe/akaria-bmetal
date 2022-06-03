/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_LOCK_H_
#define BAREMETAL_CRT_ARCH_GEN_LOCK_H_

#include <bmetal/bmetal.h>

#ifndef __arch_spinlock_t
typedef struct __arch_gen_spinlock {
	int val;
	int id;
} __arch_gen_spinlock_t;
#define __arch_spinlock_t    __arch_gen_spinlock_t
#endif

#ifndef __arch_spinlock_init
#define __arch_spinlock_init    __arch_gen_spinlock_init
int __arch_gen_spinlock_init(__arch_spinlock_t *lock);
#endif

#ifndef __arch_spinlock_lock
#define __arch_spinlock_lock    __arch_gen_spinlock_lock
int __arch_gen_spinlock_lock(__arch_spinlock_t *lock);
#endif

#ifndef __arch_spinlock_trylock
#define __arch_spinlock_trylock    __arch_gen_spinlock_trylock
int __arch_gen_spinlock_trylock(__arch_spinlock_t *lock);
#endif

#ifndef __arch_spinlock_unlock
#define __arch_spinlock_unlock    __arch_gen_spinlock_unlock
int __arch_gen_spinlock_unlock(__arch_spinlock_t *lock);
#endif

#endif /* BAREMETAL_CRT_ARCH_GEN_LOCK_H_ */
