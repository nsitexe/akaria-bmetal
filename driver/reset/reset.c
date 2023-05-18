/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/drivers/reset.h>
#include <bmetal/device.h>

static struct __reset_device *reset_system;

struct __reset_device *__reset_get_system(void)
{
	return reset_system;
}

int __reset_set_system(struct __reset_device *reset)
{
	reset_system = reset;

	return 0;
}

int __reset_add_device(struct __reset_device *reset, struct __bus *parent)
{
	struct __device *dev = __reset_to_dev(reset);
	uint32_t val;
	int r;

	r = __device_add(__reset_to_dev(reset), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	r = __device_read_conf_u32(dev, "system", &val, 0);
	if (r) {
		/* Not found, default value is 0 */
		val = 0;
	}
	if (val) {
		__reset_set_system(reset);
	}

	return 0;
}

int __reset_remove_device(struct __reset_device *reset)
{
	return __device_remove(__reset_to_dev(reset));
}
