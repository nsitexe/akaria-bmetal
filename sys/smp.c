/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/smp.h>
#include <bmetal/lock.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>

static struct __spinlock lock_smp;

int __smp_lock(void)
{
	__spinlock_lock(&lock_smp);

	return 0;
}

int __smp_unlock(void)
{
	__spinlock_unlock(&lock_smp);

	return 0;
}

/**
 * Need to hold smp_lock.
 */
int __smp_find_idle_cpu(struct __cpu_device **cpu_idle)
{
	struct __cpu_device *cpu = NULL;
	int found = 0, id;

	for (id = 0; id < CONFIG_NUM_CORES; id++) {
		cpu = __cpu_get(id);

		if (cpu && !cpu->ti_task) {
			found = 1;
			break;
		}
	}
	if (!found) {
		pri_warn("smp: Not found idle cpu.\n");
		return -ENOMEM;
	}

	if (cpu_idle) {
		*cpu_idle = cpu;
	}

	return 0;
}
