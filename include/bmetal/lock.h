/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_LOCK_H_
#define BAREMETAL_CRT_LOCK_H_

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>

#if !defined(__ASSEMBLER__)

struct __spinlock {
	k_arch_spinlock_t lock;
};

int __spinlock_init(struct __spinlock *s);
int __spinlock_lock(struct __spinlock *s);
int __spinlock_trylock(struct __spinlock *s);
int __spinlock_unlock(struct __spinlock *s);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_LOCK_H_ */
