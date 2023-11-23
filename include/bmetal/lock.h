/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_LOCK_H_
#define BAREMETAL_CRT_LOCK_H_

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>

#if !defined(__ASSEMBLER__)

struct k_spinlock {
	k_arch_spinlock_t lock;
};

int k_spinlock_init(struct k_spinlock *s);
int k_spinlock_lock(struct k_spinlock *s);
int k_spinlock_trylock(struct k_spinlock *s);
int k_spinlock_unlock(struct k_spinlock *s);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_LOCK_H_ */
