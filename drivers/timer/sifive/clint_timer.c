/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>

#include <bmetal/drivers/timer.h>
#include <bmetal/arch.h>
#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/sys/time.h>

/* Start address is 0x4000 of clint */
#define REG_MTIMECMP(hart)    (0x0000 + (hart) * 8)
#define REG_MTIME             0x7ff8

struct timer_clint_priv {
	struct __timer_device *timer;
	struct __clk_device *clk_in;
	int index_clk_in;
	uint64_t freq_in;
};
CHECK_PRIV_SIZE_TIMER(struct timer_clint_priv);

static int timer_clint_get_freq(struct __timer_device *tm, int index, uint64_t *freq)
{
	struct __device *dev = __timer_to_dev(tm);
	struct timer_clint_priv *priv = dev->priv;

	if (priv->freq_in == 0) {
		__dev_err(dev, "input clock frequency is 0.\n");
		return -EINVAL;
	}

	if (freq) {
		*freq = priv->freq_in;
	}

	return 0;
}

static int timer_clint_get_raw(struct __timer_device *tm, int index, uint64_t *count)
{
	struct __device *dev = __timer_to_dev(tm);
	uint64_t v;

#ifdef CONFIG_64BIT
	v = __device_read64(dev, REG_MTIME);
#else
	uint32_t vh1, vh2, vl;

	do {
		vh1 = __device_read64(dev, REG_MTIME);
		vl = __device_read64(dev, REG_MTIME);
		vh2 = __device_read64(dev, REG_MTIME);
	} while (vh1 != vh2);

	v = (vh1 << 32) | vl;
#endif /* CONFIG_64BIT */

	if (count) {
		*count = v;
	}

	return 0;
}

static int timer_clint_add(struct __device *dev)
{
	struct timer_clint_priv *priv = dev->priv;
	struct __timer_device *timer = __timer_from_dev(dev);
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->timer = timer;

	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	r = __clk_get_clk_from_config(dev, 0, &priv->clk_in, &priv->index_clk_in);
	if (r) {
		return r;
	}

	r = __clk_get_frequency(priv->clk_in, priv->index_clk_in, &priv->freq_in);
	if (r) {
		__dev_err(dev, "clock freq is unknown.\n");
		return r;
	}

	r = __clk_enable(priv->clk_in, priv->index_clk_in);
	if (r) {
		return r;
	}

	return 0;
}

static int timer_clint_remove(struct __device *dev)
{
	return 0;
}

const static struct __device_driver_ops timer_clint_dev_ops = {
	.add = timer_clint_add,
	.remove = timer_clint_remove,
	.mmap = __device_driver_mmap,
};

const static struct __timer_driver_ops timer_clint_timer_ops = {
	.get_freq = timer_clint_get_freq,
	.get_raw = timer_clint_get_raw,
};

static struct __timer_driver timer_clint_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "clint0_timer",
		},

		.ops = &timer_clint_dev_ops,
	},

	.ops = &timer_clint_timer_ops,
};

static int timer_clint_init(void)
{
	__timer_add_driver(&timer_clint_drv);

	return 0;
}
define_init_func(timer_clint_init);
