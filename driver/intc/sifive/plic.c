/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/intc.h>
#include <bmetal/device.h>
#include <bmetal/event.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>

#define PLIC_SOURCES_MAX         64
#define PLIC_CONTEXTS_MAX        64

#define REG_PRIORITY(n)          (0x000000 + (n) * 0x4)
#define REG_PENDING(n)           (0x001000 + (n) / 32 * 0x4)
#define REG_INTR_EN(n, ctx)      (0x002000 + (ctx) * 0x80 + (n) / 32 * 0x4)
#define REG_PRIORITY_THR(ctx)    (0x200000 + (ctx) * 0x1000 + 0x0)
#define REG_CLAIM(ctx)           (0x200000 + (ctx) * 0x1000 + 0x4)

struct intc_plic_priv {
	struct __intc_device *intc;
	uint32_t n_event;
	uint32_t n_ctx;

	struct __event_handler *hnd[PLIC_SOURCES_MAX];
};
CHECK_PRIV_SIZE_INTC(struct intc_plic_priv);

/**
 * Clear all registers that have undefined value after reset.
 *
 * @param intc A pointer of PLIC device.
 * @param n_event Number of interrupt sources.
 * @param n_ctx Number of contexts.
 * @return 0 if success, not 0 if failed.
 */
static int intc_plic_clear_all(struct __intc_device *intc, int n_event, int n_ctx)
{
	struct __device *dev = __intc_to_dev(intc);

	for (int ctx = 0; ctx < n_ctx; ctx++) {
		__device_write32(dev, 0, REG_PRIORITY_THR(ctx));

		for (int event = 0; event < n_event; event++) {
			__device_write32(dev, 0, REG_PRIORITY(event));
		}

		for (int event = 0; event < n_event; event += 32) {
			__device_write32(dev, 0, REG_INTR_EN(event, ctx));
		}
	}

	return 0;
}

static int intc_plic_enable_interrupt(struct __intc_device *intc, int event, int ctx)
{
	struct __device *dev = __intc_to_dev(intc);
	int bpos = event & 0x1f;
	uint32_t tmp;

	//TODO: use priority, currently always use lowest priority
	__device_write32(dev, 1, REG_PRIORITY(event));
	__device_write32(dev, 0, REG_PRIORITY_THR(ctx));

	tmp = __device_read32(dev, REG_INTR_EN(event, ctx));
	tmp |= 1UL << bpos;
	__device_write32(dev, tmp, REG_INTR_EN(event, ctx));

	return 0;
}

static int intc_plic_disable_interrupt(struct __intc_device *intc, int event, int ctx)
{
	struct __device *dev = __intc_to_dev(intc);
	int bpos = event & 0x1f;
	uint32_t tmp;

	tmp = __device_read32(dev, REG_INTR_EN(event, ctx));
	tmp &= ~(1UL << bpos);
	__device_write32(dev, tmp, REG_INTR_EN(event, ctx));

	return 0;
}

static int intc_plic_add_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	struct __device *dev = __intc_to_dev(intc);
	struct intc_plic_priv *priv = dev->priv;
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}
	if (event < 0 || PLIC_SOURCES_MAX <= event) {
		__dev_err(dev, "event %d is too large.\n", event);
		return -EINVAL;
	}
	if (priv->hnd[event]) {
		__dev_err(dev, "event %d is already existed.\n", event);
		return -EINVAL;
	}

	priv->hnd[event] = handler;

	//TODO: how to manage multiple contexts??
	int ctx = 0;

	r = intc_plic_enable_interrupt(intc, event, ctx);
	if (r) {
		return r;
	}

	return 0;
}

static int intc_plic_remove_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	struct __device *dev = __intc_to_dev(intc);
	struct intc_plic_priv *priv = dev->priv;
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}
	if (event < 0 || PLIC_SOURCES_MAX <= event) {
		__dev_err(dev, "event %d is too large.\n", event);
		return -EINVAL;
	}
	if (!priv->hnd[event]) {
		__dev_err(dev, "event %d is not used.\n", event);
		return -EINVAL;
	}

	//TODO: how to manage multiple contexts??
	int ctx = 0;

	r = intc_plic_disable_interrupt(intc, event, ctx);
	if (r) {
		return r;
	}

	priv->hnd[event] = NULL;

	return 0;
}

static int intc_plic_intr(int event, struct __event_handler *hnd)
{
	struct intc_plic_priv *priv = hnd->priv;
	struct __device *dev = __intc_to_dev(priv->intc);
	uint32_t v;
	int r = 0, res = EVENT_NOT_HANDLED;

	//TODO: how to manage multiple contexts??
	int ctx = 0;

	v = __device_read32(dev, REG_CLAIM(ctx));
	if (priv->hnd[v]) {
		r = __event_handle_generic(v, priv->hnd[v]);
		if (r == EVENT_HANDLED) {
			__device_write32(dev, v, REG_CLAIM(ctx));
			res = EVENT_HANDLED;
		}
	}

	return res;
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

		r = __event_alloc_handler(&hnd);
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

	priv->n_event = PLIC_SOURCES_MAX;
	r = __device_read_conf_u32(dev, "n_source", &priv->n_event, 0);
	if (r) {
		__dev_info(dev, "config 'n_source' is not found, use default:%d.\n", PLIC_SOURCES_MAX);
	}
	priv->n_event = NMIN(PLIC_SOURCES_MAX, priv->n_event);

	priv->n_ctx = PLIC_CONTEXTS_MAX;
	r = __device_read_conf_u32(dev, "n_context", &priv->n_ctx, 0);
	if (r) {
		__dev_info(dev, "config 'n_context' is not found, use default:%d.\n", PLIC_CONTEXTS_MAX);
	}
	priv->n_ctx = NMIN(PLIC_CONTEXTS_MAX, priv->n_ctx);

	r = intc_plic_clear_all(intc, priv->n_event, priv->n_ctx);
	if (r) {
		return r;
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
