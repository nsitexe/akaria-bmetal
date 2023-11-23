/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_LOCK_H_
#define BAREMETAL_CRT_ARCH_GEN_LOCK_H_

#include <bmetal/bmetal.h>
#include <bmetal/atomic.h>

#if !defined(__ASSEMBLER__)

#ifndef k_arch_spinlock_t
typedef struct k_arch_gen_spinlock {
	__atomic_int val;
	int id;
} k_arch_gen_spinlock_t;
#define k_arch_spinlock_t    k_arch_gen_spinlock_t
#endif

#ifndef k_arch_spinlock_init
#define k_arch_spinlock_init    k_arch_gen_spinlock_init
int k_arch_gen_spinlock_init(k_arch_spinlock_t *lock);
#endif

#ifndef k_arch_spinlock_lock
#define k_arch_spinlock_lock    k_arch_gen_spinlock_lock
int k_arch_gen_spinlock_lock(k_arch_spinlock_t *lock);
#endif

#ifndef k_arch_spinlock_trylock
#define k_arch_spinlock_trylock    k_arch_gen_spinlock_trylock
int k_arch_gen_spinlock_trylock(k_arch_spinlock_t *lock);
#endif

#ifndef k_arch_spinlock_unlock
#define k_arch_spinlock_unlock    k_arch_gen_spinlock_unlock
int k_arch_gen_spinlock_unlock(k_arch_spinlock_t *lock);
#endif

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_GEN_LOCK_H_ */
