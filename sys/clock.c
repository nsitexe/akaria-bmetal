/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/clock.h>
#include <bmetal/printk.h>
#include <bmetal/driver/cpu.h>
#include <bmetal/driver/timer.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/time.h>

static struct timespec64 ts_realtime_off;

int k_clock_raw_to_timespec(uint64_t count, uint64_t freq, struct timespec64 *tsp)
{
	if (!tsp) {
		return -EFAULT;
	}

	tsp->tv_sec = count / freq;
	count -= tsp->tv_sec * freq;
	tsp->tv_nsec = count * 1000000000ULL / freq;

	return 0;
}

int k_clock_timespec_to_raw(const struct timespec64 *tsp, uint64_t freq, uint64_t *count)
{
	uint64_t v;

	if (!tsp) {
		return -EFAULT;
	}

	v = tsp->tv_sec * freq;
	v += tsp->tv_nsec * freq / 1000000000ULL;

	if (count) {
		*count = v;
	}

	return 0;
}

int k_clock_get_realtime(struct timespec64 *tsp)
{
	struct timespec64 mono;
	int r;

	r = k_clock_get_monotonic(&mono);
	if (r) {
		return r;
	}

	if (tsp) {
		timespecadd(&ts_realtime_off, &mono, tsp);
	}

	return 0;
}

int k_clock_set_realtime(const struct timespec64 *tsp)
{
	struct timespec64 mono;
	int r;

	if (!tsp) {
		return -EFAULT;
	}

	r = k_clock_get_monotonic(&mono);
	if (r) {
		return r;
	}

	if (tsp) {
		timespecsub(tsp, &mono, &ts_realtime_off);
	}

	return 0;
}

int k_clock_get_monotonic(struct timespec64 *tsp)
{
	struct k_timer_device *tm = k_timer_get_default();
	const struct k_timer_driver *drv = k_timer_get_drv(tm);
	int id, r;

	if (!tm) {
		k_pri_info("system timer is not found.\n");
		return -ENOTSUP;
	}

	id = k_cpu_get_current_id_phys();

	if (drv && drv->ops->get_freq && drv->ops->get_raw) {
		struct timespec64 tmp;
		uint64_t cnt, freq;

		r = drv->ops->get_raw(tm, id, &cnt);
		if (r) {
			return r;
		}

		r = drv->ops->get_freq(tm, id, &freq);
		if (r) {
			return r;
		}

		r = k_clock_raw_to_timespec(cnt, freq, &tmp);
		if (r) {
			return r;
		}

		if (tsp) {
			*tsp = tmp;
		}
	}

	return 0;
}

int k_clock_on_tick(void)
{
	struct k_timer_device *tm = k_timer_get_default();
	const struct k_timer_driver *drv = k_timer_get_drv(tm);
	int id, r;

	if (!tm) {
		k_pri_info("system timer is not found.\n");
		return -ENOTSUP;
	}

	id = k_cpu_get_current_id_phys();

	if (drv && drv->ops->set_trigger) {
		struct timespec64 cur, tick, next;
		uint64_t hz = 100;

		r = k_clock_get_monotonic(&cur);
		if (r) {
			return r;
		}

#ifdef CONFIG_SYSTEM_TIMER_HZ
		hz = CONFIG_SYSTEM_TIMER_HZ;
#endif
		tick.tv_sec = 0;
		tick.tv_nsec = 1000000000ULL / hz;
		timespecadd(&cur, &tick, &next);

		r = drv->ops->set_trigger(tm, id, &next);
		if (r) {
			return r;
		}
	}

	return 0;
}
