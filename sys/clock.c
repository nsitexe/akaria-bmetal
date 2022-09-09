/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/clock.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/timer.h>
#include <bmetal/sys/time.h>

int __clock_get_monotonic(struct timespec64 *tsp)
{
	struct __timer_device *tm = __timer_get_system();
	const struct __timer_driver *drv = __timer_get_drv(tm);
	struct timespec64 tmp;
	int r;

	if (!tm) {
		printk("system timer is not found.\n");
		return -ENOTSUP;
	}

	if (drv && drv->ops->get_freq && drv->ops->get_raw) {
		uint64_t freq, cnt;

		r = drv->ops->get_freq(tm, 0, &freq);
		if (r) {
			return r;
		}

		r = drv->ops->get_raw(tm, 0, &cnt);
		if (r) {
			return r;
		}

		tmp.tv_sec = cnt / freq;
		cnt -= tmp.tv_sec * freq;
		tmp.tv_nsec = cnt * 1000000000ULL / freq;

		if (tsp) {
			*tsp = tmp;
		}
	}

	return 0;
}
