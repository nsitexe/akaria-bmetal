/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/fini.h>
#include <bmetal/printk.h>
#include <bmetal/driver/reset.h>

int k_fini_reboot(void)
{
	struct k_reset_device *rst = k_reset_get_system();
	const struct k_reset_driver *drv = k_reset_get_drv(rst);
	int r;

	if (!rst) {
		k_pri_info("system reset is not found.\n");
		return -ENOTSUP;
	}

	if (drv && drv->ops->reboot) {
		r = drv->ops->reboot(rst);
		if (r) {
			return r;
		}
	}

	return 0;
}

int k_fini_power_off(void)
{
	struct k_reset_device *rst = k_reset_get_system();
	const struct k_reset_driver *drv = k_reset_get_drv(rst);
	int r;

	if (!rst) {
		k_pri_info("system reset is not found.\n");
		return -ENOTSUP;
	}

	if (drv && drv->ops->power_off) {
		r = drv->ops->power_off(rst);
		if (r) {
			return r;
		}
	}

	return 0;
}
