/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/driver/reset.h>
#include <bmetal/device.h>

static struct k_reset_device *reset_system;

struct k_reset_device *k_reset_get_system(void)
{
	return reset_system;
}

int k_reset_set_system(struct k_reset_device *reset)
{
	reset_system = reset;

	return 0;
}

int k_reset_add_device(struct k_reset_device *reset, struct k_bus *parent)
{
	struct k_device *dev = k_reset_to_dev(reset);
	uint32_t val;
	int r;

	r = k_device_add(k_reset_to_dev(reset), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	r = k_device_read_conf_u32(dev, "system", &val, 0);
	if (r) {
		/* Not found, default value is 0 */
		val = 0;
	}
	if (val) {
		k_reset_set_system(reset);
	}

	return 0;
}

int k_reset_remove_device(struct k_reset_device *reset)
{
	return k_device_remove(k_reset_to_dev(reset));
}
