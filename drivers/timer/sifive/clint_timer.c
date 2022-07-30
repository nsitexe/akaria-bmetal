/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>

#include <bmetal/drivers/timer.h>
#include <bmetal/arch.h>
#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>

/* Start address is 0x4000 of clint */
#define REG_MTIMECMP(hart)    (0x0000 + (hart) * 8)
#define REG_MTIME             0x3ff8

struct timer_clint_priv {
	struct __timer_device *timer;
};
CHECK_PRIV_SIZE_TIMER(struct timer_clint_priv);

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
