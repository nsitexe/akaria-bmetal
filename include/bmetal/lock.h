/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_LOCK_H_
#define BAREMETAL_CRT_LOCK_H_

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>

struct __spinlock {
	__arch_spinlock_t lock;
};

int __spinlock_init(struct __spinlock *s);
int __spinlock_lock(struct __spinlock *s);
int __spinlock_trylock(struct __spinlock *s);
int __spinlock_unlock(struct __spinlock *s);

int __arch_gen_spinlock_init(__arch_spinlock_t *lock);
int __arch_gen_spinlock_lock(__arch_spinlock_t *lock);
int __arch_gen_spinlock_trylock(__arch_spinlock_t *lock);
int __arch_gen_spinlock_unlock(__arch_spinlock_t *lock);

#endif /* BAREMETAL_CRT_LOCK_H_ */
