/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/drivers/timer.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/time.h>

static struct __timer_device *system_timer;

struct __timer_device *__system_timer_get(void)
{
	return system_timer;
}

int __system_timer_set(struct __timer_device *timer)
{
	system_timer = timer;

	return 0;
}

int __timer_add_device(struct __timer_device *timer, struct __bus *parent)
{
	struct __device *dev = __timer_to_dev(timer);
	uint32_t val;
	int r;

	r = __device_add(__timer_to_dev(timer), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	r = __device_read_conf_u32(dev, "system", &val, 0);
	if (r) {
		/* Not found, default value is 0 */
		val = 0;
	}
	if (val) {
		__system_timer_set(timer);
	}

	return 0;
}

int __timer_remove_device(struct __timer_device *timer)
{
	return __device_remove(__timer_to_dev(timer));
}
