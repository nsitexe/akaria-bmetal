/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/driver/intc.h>
#include <bmetal/device.h>
#include <bmetal/event.h>
#include <bmetal/init.h>
#include <bmetal/intr.h>
#include <bmetal/printk.h>
#include <bmetal/binding/intc/riscv/rv_priv.h>
#include <bmetal/driver/cpu.h>
#include <bmetal/sys/errno.h>

/* Start address is 0x0000 of clint */
#define REG_MSIP(hart)        (0x0000 + (hart) * 4)

struct intc_clint_priv {
	struct k_intc_device *intc;
};
CHECK_PRIV_SIZE_INTC(struct intc_clint_priv);

static int intc_clint_intr(int event, struct k_event_handler *hnd)
{
	struct intc_clint_priv *priv = hnd->priv;
	struct k_device *dev = k_intc_to_dev(priv->intc);
	int id_phys = k_cpu_get_current_id_phys();
	int res = EVENT_NOT_HANDLED;

	switch (event) {
	case RV_IX_MSIX:
		k_device_write32(dev, 0, REG_MSIP(id_phys));
		res = EVENT_HANDLED;
		break;
	default:
		break;
	}

	return res;
}

static int intc_clint_add(struct k_device *dev)
{
	struct intc_clint_priv *priv = dev->priv;
	struct k_intc_device *intc = k_intc_from_dev(dev);
	struct k_intc_device *intc_parent;
	int len, num_irq, r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->intc = intc;

	r = k_io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	r = k_intc_get_conf_length(dev, &len);
	if (r) {
		return r;
	}

	for (int i = 0; i < len; i++) {
		struct k_event_handler *hnd;

		r = k_intc_get_intc_from_config(dev, i, &intc_parent, &num_irq);
		if (r) {
			return r;
		}

		r = k_event_alloc_handler(&hnd);
		if (r) {
			return r;
		}

		hnd->func = intc_clint_intr;
		hnd->priv = priv;

		r = k_intc_add_handler(intc_parent, num_irq, hnd);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int intc_clint_remove(struct k_device *dev)
{
	return 0;
}

const static struct k_device_driver_ops intc_clint_dev_ops = {
	.add = intc_clint_add,
	.remove = intc_clint_remove,
	.mmap = k_device_driver_mmap,
};

static int intc_clint_raise_ipi(struct k_intc_device *intc, struct k_cpu_device *src, struct k_cpu_device *dest, void *arg)
{
	struct k_device *dev = k_intc_to_dev(intc);
	int id_cur = k_cpu_get_current_id_phys();
	int id_dest = k_cpu_get_id_phys(dest);

	if (id_cur != k_cpu_get_id_phys(src)) {
		k_dev_err(dev, "cannot send IPI, src:%d is not current CPU.\n",
			k_cpu_get_id_phys(src));
		return -EINVAL;
	}

	k_device_write32(dev, 1, REG_MSIP(id_dest));

	return 0;
}

const static struct k_intc_driver_ops intc_clint_intc_ops = {
	.raise_ipi = intc_clint_raise_ipi,
};

static struct k_intc_driver intc_clint_drv = {
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
	k_intc_add_driver(&intc_clint_drv);

	return 0;
}
define_init_func(intc_clint_init);
