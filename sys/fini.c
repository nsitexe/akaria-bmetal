/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/fini.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/reset.h>

int __fini_reboot(void)
{
	struct __reset_device *rst = __reset_get_system();
	const struct __reset_driver *drv = __reset_get_drv(rst);
	int r;

	if (!rst) {
		pri_info("system reset is not found.\n");
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

int __fini_power_off(void)
{
	struct __reset_device *rst = __reset_get_system();
	const struct __reset_driver *drv = __reset_get_drv(rst);
	int r;

	if (!rst) {
		pri_info("system reset is not found.\n");
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
