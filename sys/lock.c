/* SPDX-License-Identifier: Apache-2.0 */

#include <stdatomic.h>

#include <bmetal/lock.h>

int __spinlock_init(struct __spinlock *s)
{
	return __arch_spinlock_init(&s->lock);
}

int __spinlock_lock(struct __spinlock *s)
{
	return __arch_spinlock_lock(&s->lock);
}

int __spinlock_trylock(struct __spinlock *s)
{
	return __arch_spinlock_trylock(&s->lock);
}

int __spinlock_unlock(struct __spinlock *s)
{
	return __arch_spinlock_unlock(&s->lock);
}

int __arch_gen_spinlock_init(__arch_spinlock_t *lock)
{
	lock->val = 0;

	return 0;
}

int __arch_gen_spinlock_lock(__arch_spinlock_t *lock)
{
	int r = 1;

	while (r) {
		r = __arch_gen_spinlock_trylock(lock);
	}
	lock->id = __arch_get_cpu_id();

	return 0;
}

int __arch_gen_spinlock_trylock(__arch_spinlock_t *lock)
{
	int tmp = 1, val;

	val = atomic_exchange(&lock->val, tmp);

	return val;
}

int __arch_gen_spinlock_unlock(__arch_spinlock_t *lock)
{
	lock->val = 0;
	lock->id = -1;

	return 0;
}
