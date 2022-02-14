/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/clk.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>

int __clk_add_device(struct __clk_device *clk, struct __bus *parent)
{
	int r;

	r = __device_add(__clk_to_dev(clk), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	return 0;
}

int __clk_remove_device(struct __clk_device *clk)
{
	return __device_remove(__clk_to_dev(clk));
}
