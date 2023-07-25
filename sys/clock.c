/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/clock.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/timer.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/time.h>

static struct timespec64 ts_realtime_off;

int __clock_raw_to_timespec(uint64_t count, uint64_t freq, struct timespec64 *tsp)
{
	if (!tsp) {
		return -EFAULT;
	}

	tsp->tv_sec = count / freq;
	count -= tsp->tv_sec * freq;
	tsp->tv_nsec = count * 1000000000ULL / freq;

	return 0;
}

int __clock_timespec_to_raw(const struct timespec64 *tsp, uint64_t freq, uint64_t *count)
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

int __clock_get_realtime(struct timespec64 *tsp)
{
	struct timespec64 mono;
	int r;

	r = __clock_get_monotonic(&mono);
	if (r) {
		return r;
	}

	if (tsp) {
		timespecadd(&ts_realtime_off, &mono, tsp);
	}

	return 0;
}

int __clock_set_realtime(const struct timespec64 *tsp)
{
	struct timespec64 mono;
	int r;

	if (!tsp) {
		return -EFAULT;
	}

	r = __clock_get_monotonic(&mono);
	if (r) {
		return r;
	}

	if (tsp) {
		timespecsub(tsp, &mono, &ts_realtime_off);
	}

	return 0;
}

int __clock_get_monotonic(struct timespec64 *tsp)
{
	struct __timer_device *tm = __system_timer_get();
	const struct __timer_driver *drv = __timer_get_drv(tm);
	int r;

	if (!tm) {
		pri_info("system timer is not found.\n");
		return -ENOTSUP;
	}

	if (drv && drv->ops->get_freq && drv->ops->get_raw) {
		struct timespec64 tmp;
		uint64_t cnt, freq;

		r = drv->ops->get_raw(tm, 0, &cnt);
		if (r) {
			return r;
		}

		r = drv->ops->get_freq(tm, 0, &freq);
		if (r) {
			return r;
		}

		r = __clock_raw_to_timespec(cnt, freq, &tmp);
		if (r) {
			return r;
		}

		if (tsp) {
			*tsp = tmp;
		}
	}

	return 0;
}
