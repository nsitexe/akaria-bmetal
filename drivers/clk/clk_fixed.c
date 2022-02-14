/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/clk.h>
#include <bmetal/init.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>

static int clk_fixed_add(struct __device *dev)
{
	uint32_t f;
	int r;

	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	r = __device_read_conf_u32(dev, "frequency", &f, 0);
	if (r) {
		printk("clk_fixed: config 'frequency' is not found.\n");
		return -EINVAL;
	}

	return 0;
}

static int clk_fixed_remove(struct __device *dev)
{
	return 0;
}

const static struct __device_driver_ops clk_fixed_dev_ops = {
	.add = clk_fixed_add,
	.remove = clk_fixed_remove,
};

const static struct __clk_driver_ops clk_fixed_clk_ops = {
};

static struct __clk_driver clk_fixed_drv = {
	.base = {
		.base = {
			.type_vendor = "none",
			.type_device = "clk_fixed",
		},

		.ops = &clk_fixed_dev_ops,
	},

	.ops = &clk_fixed_clk_ops,
};

static int clk_fixed_init(void)
{
	__clk_add_driver(&clk_fixed_drv);

	return 0;
}

define_init_func(clk_fixed_init);
