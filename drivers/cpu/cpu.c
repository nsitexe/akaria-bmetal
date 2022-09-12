/* SPDX-License-Identifier: Apache-2.0 */

#include <stdatomic.h>

#include <bmetal/drivers/cpu.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/sys/errno.h>

static struct __cpu_device *cpus[CONFIG_NUM_CORES];
static int uniq_id_cpu = 1;

int __cpu_get_id(struct __cpu_device *cpu)
{
	return cpu->id_cpu;
}

int __cpu_get_id_phys(struct __cpu_device *cpu)
{
	return cpu->id_phys;
}

int __cpu_get_running(struct __cpu_device *cpu)
{
	return cpu->running;
}

void __cpu_set_running(struct __cpu_device *cpu, int r)
{
	cpu->running = r;
}

struct __thread_info *__cpu_get_thread(struct __cpu_device *cpu)
{
	return cpu->ti;
}

void __cpu_set_thread(struct __cpu_device *cpu, struct __thread_info *ti)
{
	cpu->ti = ti;
}
struct __thread_info *__cpu_get_thread_idle(struct __cpu_device *cpu)
{
	return cpu->ti_idle;
}

void __cpu_set_thread_idle(struct __cpu_device *cpu, struct __thread_info *ti)
{
	cpu->ti_idle = ti;
}

struct __thread_info *__cpu_get_thread_task(struct __cpu_device *cpu)
{
	return cpu->ti_task;
}

void __cpu_set_thread_task(struct __cpu_device *cpu, struct __thread_info *ti)
{
	cpu->ti_task = ti;
}

__arch_user_regs_t *__cpu_get_user_regs(struct __cpu_device *cpu)
{
	return cpu->regs;
}

void __cpu_set_user_regs(struct __cpu_device *cpu, __arch_user_regs_t *regs)
{
	cpu->regs = regs;
}

__arch_user_regs_t *__cpu_get_current_user_regs(void)
{
	return __cpu_get_user_regs(__cpu_get_current());
}

void __cpu_set_current_user_regs(__arch_user_regs_t *regs)
{
	__cpu_set_user_regs(__cpu_get_current(), regs);
}

int __cpu_alloc_id(void)
{
	return uniq_id_cpu++;
}

struct __cpu_device *__cpu_get(int id)
{
	if (id < 0 || CONFIG_NUM_CORES <= id) {
		printk("cpu_get: id:%d is out of bounds.\n", id);
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
		printk("cpu_set: id:%d is out of bounds.\n", id);
		return -EINVAL;
	}

	cpus[id] = cpu;

	return 0;
}

struct __cpu_device *__cpu_get_by_physical_id(int id_phys)
{
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		if (!cpus[i]) {
			continue;
		}
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

int __cpu_cache_get_line_size_i(struct __cpu_device *cpu)
{
	return cpu->line_size_i;
}

void __cpu_cache_set_line_size_i(struct __cpu_device *cpu, int sz)
{
	cpu->line_size_i = sz;
}

int __cpu_cache_get_line_size_d(struct __cpu_device *cpu)
{
	return cpu->line_size_d;
}

void __cpu_cache_set_line_size_d(struct __cpu_device *cpu, int sz)
{
	cpu->line_size_d = sz;
}

int __cpu_cache_clean_range(struct __cpu_device *cpu, const void *start, size_t sz)
{
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->clean_range) {
		r = drv->ops->clean_range(cpu, start, sz);
		if (r) {
			__dev_err(__cpu_to_dev(cpu), "clean range:%p-%p failed.\n",
				start, start + sz);
			return r;
		}
	}

	return 0;
}

int __cpu_cache_inv_range(struct __cpu_device *cpu, const void *start, size_t sz)
{
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->inv_range) {
		r = drv->ops->inv_range(cpu, start, sz);
		if (r) {
			__dev_err(__cpu_to_dev(cpu), "invalidate range:%p-%p failed.\n",
				start, start + sz);
			return r;
		}
	}

	return 0;
}

int __cpu_cache_flush_range(struct __cpu_device *cpu, const void *start, size_t sz)
{
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->flush_range) {
		r = drv->ops->flush_range(cpu, start, sz);
		if (r) {
			__dev_err(__cpu_to_dev(cpu), "flush range:%p-%p failed.\n",
				start, start + sz);
			return r;
		}
	}

	return 0;
}

static int __cpu_call_event_handler(struct __cpu_device *cpu, enum __cpu_event ev)
{
	int r, res = 0;

	if (CPU_EVENT_MAX <= ev) {
		__dev_err(__cpu_to_dev(cpu), "cpu event %d is unknown.\n", ev);
		return -EINVAL;
	}

	if (cpu->handlers[ev] && cpu->handlers[ev]->func) {
		r = cpu->handlers[ev]->func(ev, cpu->handlers[ev]);
		if (r != EVENT_HANDLED) {
			res = -EINVAL;
		}
	}

	return res;
}

int __cpu_get_event_handler(struct __cpu_device *cpu, enum __cpu_event ev, struct __event_handler **hnd)
{
	if (CPU_EVENT_MAX <= ev) {
		__dev_err(__cpu_to_dev(cpu), "cpu event %d is unknown.\n", ev);
		return -EINVAL;
	}

	if (hnd) {
		*hnd = cpu->handlers[ev];
	}

	return 0;
}

int __cpu_set_event_handler(struct __cpu_device *cpu, enum __cpu_event ev, struct __event_handler *hnd)
{
	if (CPU_EVENT_MAX <= ev) {
		__dev_err(__cpu_to_dev(cpu), "cpu event %d is unknown.\n", ev);
		return -EINVAL;
	}

	cpu->handlers[ev] = hnd;

	return 0;
}

int __cpu_wakeup(struct __cpu_device *cpu)
{
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->wakeup) {
		r = drv->ops->wakeup(cpu);
		if (r) {
			__dev_err(__cpu_to_dev(cpu), "wakeup id:%d(phys:%d) failed.\n",
				cpu->id_cpu, cpu->id_phys);
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
			__dev_err(__cpu_to_dev(cpu), "sleep id:%d(phys:%d) failed.\n",
				cpu->id_cpu, cpu->id_phys);
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

		if (!cpu) {
			continue;
		}

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

		if (!cpu) {
			continue;
		}

		r = __cpu_sleep(cpu);
		if (r) {
			res = r;
		}
	}

	return res;
}

int __cpu_on_wakeup(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->on_wakeup) {
		r = drv->ops->on_wakeup(cpu);
		if (r) {
			__dev_err(__cpu_to_dev(cpu), "failed to callback on_wakeup.\n");
			return r;
		}
	}

	r = __cpu_call_event_handler(cpu, CPU_EVENT_ON_WAKEUP);
	if (r) {
		__dev_err(__cpu_to_dev(cpu), "failed to handle event of on_wakeup.\n");
		return r;
	}

	return 0;
}

int __cpu_on_sleep(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	r = __cpu_call_event_handler(cpu, CPU_EVENT_ON_SLEEP);
	if (r) {
		__dev_err(__cpu_to_dev(cpu), "failed to handle event of on_sleep.\n");
		return r;
	}

	if (drv && drv->ops && drv->ops->on_sleep) {
		r = drv->ops->on_sleep(cpu);
		if (r) {
			__dev_err(__cpu_to_dev(cpu), "failed to callback on_sleep.\n");
			return r;
		}
	}

	return 0;
}

int __cpu_wait_interrupt(void)
{
	__arch_wait_interrupt();

	return 0;
}

int __cpu_raise_ipi(struct __cpu_device *dest, void *arg)
{
	struct __cpu_device *cpu = __cpu_get_current();
	const struct __cpu_driver *drv = __cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->raise_ipi) {
		r = drv->ops->raise_ipi(cpu, dest, arg);
		if (r) {
			__dev_err(__cpu_to_dev(cpu), "failed to raise IPI.\n");
			return r;
		}
	} else {
		__dev_err(__cpu_to_dev(cpu), "not supported to raise IPI.\n");
		return -ENOTSUP;
	}

	return 0;
}

int __cpu_futex_wait(int *uaddr, int val, int bitset)
{
	struct __cpu_device *cpu = __cpu_get_current();
	long st;
	int res = 0;

	if (!bitset) {
		__dev_err(__cpu_to_dev(cpu), "futex_wait bitmask is 0.\n");
		return -EINVAL;
	}

	cpu->futex.uaddr = uaddr;
	cpu->futex.bitset = bitset;
	cpu->futex.wakeup = 0;

	dwmb();

	/* Avoid spurious wake up */
	while (!cpu->futex.wakeup) {
		/*
		 * Pending IPI from "uaddr != val" to wait for interrupt(wfi).
		 * If accept IPI before wfi IPI is cleared by interrupt handler
		 * and wfi will never been returned.
		 */
		__intr_save_local(&st);
		if (atomic_load((atomic_int *)uaddr) != val) {
			res = -EWOULDBLOCK;
			break;
		}
		__smp_unlock();

		__cpu_wait_interrupt();

		__smp_lock();
		__intr_restore_local(st);

		drmb();
	}

	cpu->futex.uaddr = 0;
	cpu->futex.bitset = 0;

	dwmb();

	return res;
}

int __cpu_futex_wake(int *uaddr, int val, int bitset)
{
	struct __cpu_device *cpu = __cpu_get_current();
	int r, res = 0;

	if (!bitset) {
		__dev_err(__cpu_to_dev(cpu), "futex_wake bitmask is 0.\n");
		return -EINVAL;
	}

	drmb();

	for (int i = 0; i < CONFIG_NUM_CORES && res < val; i++) {
		struct __cpu_device *cpu = __cpu_get(i);

		if (!cpu) {
			continue;
		}

		if (cpu->futex.uaddr == uaddr) {
			if (!(cpu->futex.bitset & bitset)) {
				continue;
			}

			cpu->futex.uaddr = 0;
			cpu->futex.bitset = 0;
			cpu->futex.wakeup = 1;

			dwmb();

			r = __cpu_raise_ipi(cpu, NULL);
			if (r) {
				return r;
			}

			res++;
		}
	}

	return res;
}

int __cpu_get_cpu_from_config(struct __device *dev, int index, struct __cpu_device **cpu)
{
	const char *cpu_name;
	struct __device *tmp;
	int r;

	r = __device_read_conf_str(dev, "cpu", &cpu_name, index);
	if (r) {
		__dev_err(dev, "cpu name is not found, index:%d.\n", index);
		return -EINVAL;
	}

	r = __bus_find_device(__bus_get_root(), cpu_name, &tmp);
	if (r == -EAGAIN) {
		return -EAGAIN;
	} else if (r) {
		__dev_err(dev, "cpu '%s' is not found.\n", cpu_name);
		return -EINVAL;
	}

	if (cpu) {
		*cpu = __cpu_from_dev(tmp);
	}

	return 0;
}
