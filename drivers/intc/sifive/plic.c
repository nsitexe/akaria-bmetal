/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/intc.h>
#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>

struct intc_plic_priv {
	struct __intc_device *intc;
};
CHECK_PRIV_SIZE_INTC(struct intc_plic_priv);

static int intc_plic_intr(int event, struct __event_handler *hnd)
{
	struct intc_plic_priv *priv = hnd->priv;
	//struct __device *dev = __intc_to_dev(priv->intc);
	//int id_phys = __arch_get_cpu_id();

	//switch (event) {
	//case RV_IX_MSIX:
		//__device_write32(dev, 0, REG_MSIP(id_phys));
		//break;
	//}

	return __intc_handle_generic_event(priv->intc, event, hnd->hnd_next);
}

static int intc_plic_add_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	return 0;
}

static int intc_plic_remove_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	return 0;
}

static int intc_plic_add(struct __device *dev)
{
	struct intc_plic_priv *priv = dev->priv;
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

		hnd->func = intc_plic_intr;
		hnd->priv = priv;

		r = __intc_add_handler(intc_parent, num_irq, hnd);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int intc_plic_remove(struct __device *dev)
{
	return 0;
}

const static struct __device_driver_ops intc_plic_dev_ops = {
	.add = intc_plic_add,
	.remove = intc_plic_remove,
	.mmap = __device_driver_mmap,
};

const static struct __intc_driver_ops intc_plic_intc_ops = {
	.add_handler = intc_plic_add_handler,
	.remove_handler = intc_plic_remove_handler,
};

static struct __intc_driver intc_plic_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "plic0",
		},

		.ops = &intc_plic_dev_ops,
	},

	.ops = &intc_plic_intc_ops,
};

static int intc_plic_init(void)
{
	__intc_add_driver(&intc_plic_drv);

	return 0;
}
define_init_func(intc_plic_init);
