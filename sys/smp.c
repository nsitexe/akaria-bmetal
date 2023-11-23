/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/smp.h>
#include <bmetal/lock.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>

static struct k_spinlock lock_smp;

int k_smp_lock(void)
{
	k_spinlock_lock(&lock_smp);

	return 0;
}

int k_smp_unlock(void)
{
	k_spinlock_unlock(&lock_smp);

	return 0;
}

/**
 * Need to hold smp_lock.
 */
int k_smp_find_idle_cpu(struct k_cpu_device **cpu_idle)
{
	struct k_cpu_device *cpu = NULL;
	int found = 0, id;

	for (id = 0; id < CONFIG_NUM_CORES; id++) {
		cpu = k_cpu_get(id);

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
