/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>

#include <bmetal/drivers/intc.h>
#include <bmetal/arch.h>
#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>
#include <bmetal/bindings/intc/riscv/rv_priv.h>

#define REG_MSIP(hart)        (0x0000 + (hart) * 4)
#define REG_MTIMECMP(hart)    (0x4000 + (hart) * 8)
#define REG_MTIME             0xbff8

struct intc_clint_priv {
	struct __intc_device *intc;
};
CHECK_PRIV_SIZE_INTC(struct intc_clint_priv);

static int intc_clint_intr(int event, struct __event_handler *hnd)
{
	struct intc_clint_priv *priv = hnd->priv;
	struct __device *dev = __intc_to_dev(priv->intc);
	int id_phys = __arch_get_cpu_id();

	switch (event) {
	case RV_IX_MSIX:
		__device_write32(dev, 0, REG_MSIP(id_phys));
		break;
	}

	return __intc_handle_generic_event(priv->intc, event, hnd->hnd_next);
}

static int intc_clint_add(struct __device *dev)
{
	struct intc_clint_priv *priv = dev->priv;
	struct __intc_device *intc = __intc_from_dev(dev);
	struct __intc_device *intc_parent;
	int len, num_irq, r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->intc = intc;

	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	r = __intc_get_conf_length(dev, &len);
	if (r) {
		return r;
	}

	for (int i = 0; i < len; i++) {
		struct __event_handler *hnd;

		r = __intc_get_intc_from_config(dev, i, &intc_parent, &num_irq);
		if (r) {
			return r;
		}

		r = __device_alloc_event_handler(dev, &hnd);
		if (r) {
			return r;
		}

		hnd->func = intc_clint_intr;
		hnd->priv = priv;

		r = __intc_add_handler(intc_parent, num_irq, hnd);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int intc_clint_remove(struct __device *dev)
{
	return 0;
}

const static struct __device_driver_ops intc_clint_dev_ops = {
	.add = intc_clint_add,
	.remove = intc_clint_remove,
	.mmap = __device_driver_mmap,
};

const static struct __intc_driver_ops intc_clint_intc_ops = {
};

static struct __intc_driver intc_clint_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "clint0",
		},

		.ops = &intc_clint_dev_ops,
	},

	.ops = &intc_clint_intc_ops,
};

static int intc_clint_init(void)
{
	__intc_add_driver(&intc_clint_drv);

	return 0;
}
define_init_func(intc_clint_init);
