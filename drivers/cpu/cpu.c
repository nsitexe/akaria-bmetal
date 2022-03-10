/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>

#include <bmetal/drivers/cpu.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>

static struct __cpu_device *cpus[CONFIG_NUM_CORES];
static int uniq_id_cpu = 1;

int __cpu_alloc_id(void)
{
	return uniq_id_cpu++;
}

struct __cpu_device *__cpu_get(int id)
{
	if (id < 0 || CONFIG_NUM_CORES <= id) {
		printk("cpu_get: id:%d is invalid.\n", id);
		return NULL;
	}

	return cpus[id];
}

int __cpu_set(int id, struct __cpu_device *cpu)
{
	if (!cpu) {
		return -EINVAL;
	}
	if (id < 0 || CONFIG_NUM_CORES <= id) {
		printk("cpu_set: id:%d is invalid.\n", id);
		return -EINVAL;
	}

	cpus[id] = cpu;

	return 0;
}

struct __cpu_device *__cpu_get_by_physical_id(int id_phys)
{
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		if (cpus[i]->id_phys == id_phys) {
			return cpus[i];
		}
	}
	printk("cpu_get_by_physical_id: id_phys:%d is not found.\n", id_phys);

	return NULL;
}

struct __cpu_device *__cpu_get_current(void)
{
	return __cpu_get_by_physical_id(__arch_get_cpu_id());
}

int __cpu_get_id(struct __cpu_device *cpu)
{
	return cpu->id_cpu;
}

struct __thread_info *__cpu_get_thread(struct __cpu_device *cpu)
{
	return cpu->ti;
}

int __cpu_add_device(struct __cpu_device *cpu, struct __bus *parent)
{
	int r;

	r = __device_add(__cpu_to_dev(cpu), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	return 0;
}

int __cpu_remove(struct __cpu_device *cpu)
{
	return __device_remove(__cpu_to_dev(cpu));
}

int __cpu_wakeup(struct __cpu_device *cpu)
{
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->wakeup) {
		r = drv->ops->wakeup(cpu);
		if (r) {
			printk("cpu_wakeup: id:%d(phys:%d) failed to wakeup.\n", cpu->id_cpu, cpu->id_phys);
			return r;
		}
	}

	return 0;
}

int __cpu_sleep(struct __cpu_device *cpu)
{
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->sleep) {
		r = drv->ops->sleep(cpu);
		if (r) {
			printk("cpu_sleep: id:%d(phys:%d) failed to sleep.\n", cpu->id_cpu, cpu->id_phys);
			return r;
		}
	}

	return 0;
}

int __cpu_wakeup_all(void)
{
	int r, res = 0;

	/* Main core (id:0) already booted, start from 1 */
	for (int i = 1; i < CONFIG_NUM_CORES; i++) {
		struct __cpu_device *cpu = __cpu_get(i);

		r = __cpu_wakeup(cpu);
		if (r) {
			res = r;
		}
	}

	return res;
}

int __cpu_sleep_all(void)
{
	int r, res = 0;

	/* Main core (id:0) cannot sleep, start from 1 */
	for (int i = 1; i < CONFIG_NUM_CORES; i++) {
		struct __cpu_device *cpu = __cpu_get(i);

		r = __cpu_sleep(cpu);
		if (r) {
			res = r;
		}
	}

	return res;
}
