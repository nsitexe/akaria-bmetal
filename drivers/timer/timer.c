/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>

#include <bmetal/drivers/timer.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>

int __timer_add_device(struct __timer_device *timer, struct __bus *parent)
{
	int r;

	r = __device_add(__timer_to_dev(timer), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	return 0;
}

int __timer_remove_device(struct __timer_device *timer)
{
	return __device_remove(__timer_to_dev(timer));
}
