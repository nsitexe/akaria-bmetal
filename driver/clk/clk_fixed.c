/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/clk.h>
#include <bmetal/init.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>

struct clk_fixed_priv {
	uint64_t freq;
};
CHECK_PRIV_SIZE_CLK(struct clk_fixed_priv);

static int clk_fixed_add(struct __device *dev)
{
	struct clk_fixed_priv *priv = dev->priv;
	uint64_t f;
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	r = __device_read_conf_u64(dev, "frequency", &f, 0);
	if (r) {
		__dev_err(dev, "config 'frequency' is not found.\n");
		return -EINVAL;
	}

	priv->freq = f;

	return 0;
}

static int clk_fixed_remove(struct __device *dev)
{
	return 0;
}

static int clk_fixed_enable(struct k_clk_device *clk, int index)
{
	/* Always enabled */
	return 0;
}

static int clk_fixed_disable(struct k_clk_device *clk, int index)
{
	/* Always enabled */
	return 0;
}

static int clk_fixed_get_freq(struct k_clk_device *clk, int index, uint64_t *freq)
{
	struct clk_fixed_priv *priv = k_clk_to_dev(clk)->priv;

	if (index != 0) {
		return -EINVAL;
	}

	*freq = priv->freq;

	return 0;
}

const static struct __device_driver_ops clk_fixed_dev_ops = {
	.add = clk_fixed_add,
	.remove = clk_fixed_remove,
};

const static struct k_clk_driver_ops clk_fixed_clk_ops = {
	.get_freq = clk_fixed_get_freq,
	.enable = clk_fixed_enable,
	.disable = clk_fixed_disable,
};

static struct k_clk_driver clk_fixed_drv = {
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
	k_clk_add_driver(&clk_fixed_drv);

	return 0;
}
define_init_func(clk_fixed_init);
