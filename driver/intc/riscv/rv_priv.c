/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/intc.h>
#include <bmetal/arch.h>
#include <bmetal/device.h>
#include <bmetal/event.h>
#include <bmetal/init.h>
#include <bmetal/printk.h>
#include <bmetal/bindings/intc/riscv/rv_priv.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/sys/errno.h>

/* mie, mip, sie, sip */
#define XIX_USIX        BIT(RV_IX_USIX)
#define XIX_SSIX        BIT(RV_IX_SSIX)
#define XIX_MSIX        BIT(RV_IX_MSIX)
#define XIX_UTIX        BIT(RV_IX_UTIX)
#define XIX_STIX        BIT(RV_IX_STIX)
#define XIX_MTIX        BIT(RV_IX_MTIX)
#define XIX_UEIX        BIT(RV_IX_UEIX)
#define XIX_SEIX        BIT(RV_IX_SEIX)
#define XIX_MEIX        BIT(RV_IX_MEIX)

#define XIX_SIX         BIT(RV_IX_SIX)
#define XIX_TIX         BIT(RV_IX_TIX)
#define XIX_EIX         BIT(RV_IX_EIX)

struct intc_priv_priv {
	struct k_intc_device *intc;
	struct k_cpu_device *cpu_parent;

	struct __event_handler hnd_ex;
	struct __event_handler hnd_tm;
	struct __event_handler hnd_sw;
	struct __event_handler hnd_wake;
	struct __event_handler hnd_sleep;

	unsigned int has_ex:1;
	unsigned int enabled_ex:1;
	unsigned int has_tm:1;
	unsigned int enabled_tm:1;
	unsigned int has_sw:1;
	unsigned int enabled_sw:1;
};
CHECK_PRIV_SIZE_INTC(struct intc_priv_priv);

static int intc_priv_intr(int event, struct __event_handler *hnd)
{
	struct __event_handler *hnd_child = NULL;
	int r, res = EVENT_NOT_HANDLED;

	/*
	 * The hnd is hnd_ex or tm or sw.
	 * We need to callback from next handler.
	 */
	switch (event) {
	case RV_CAUSE_INT_EX:
	case RV_CAUSE_INT_TM:
	case RV_CAUSE_INT_SW:
		hnd_child = hnd->hnd_next;
		break;
	default:
		break;
	}

	if (hnd_child) {
		r = __event_handle_generic(event, hnd_child);
		if (r == EVENT_HANDLED) {
			res = EVENT_HANDLED;
		}
	}

	return res;
}

static int intc_priv_update_intr_mask(struct k_intc_device *intc, int wakeup)
{
	struct __device *dev = k_intc_to_dev(intc);
	struct intc_priv_priv *priv = dev->priv;
	int m = 0;

	if (priv->has_ex && priv->enabled_ex) {
		m |= XIX_EIX;
	}
	if (priv->has_tm && priv->enabled_tm) {
		m |= XIX_TIX;
	}

	/* Always set SW interrupt for IPI */
	m |= XIX_SIX;

	if (wakeup) {
		__asm volatile ("csrs mie, %0" : : "r"(m));
	} else {
		__asm volatile ("csrc mie, %0" : : "r"(m));
	}

	return 0;
}

static int intc_priv_cpu_event(int event, struct __event_handler *hnd)
{
	struct intc_priv_priv *priv = hnd->priv;
	struct k_intc_device *intc = priv->intc;
	int r, res = EVENT_NOT_HANDLED;

	switch (event) {
	case CPU_EVENT_ON_WAKEUP:
		r = intc_priv_update_intr_mask(intc, 1);
		if (r) {
			return EVENT_NOT_HANDLED;
		}
		res = EVENT_HANDLED;

		break;
	case CPU_EVENT_ON_SLEEP:
		r = intc_priv_update_intr_mask(intc, 0);
		if (r) {
			return EVENT_NOT_HANDLED;
		}
		res = EVENT_HANDLED;

		break;
	default:
		res = EVENT_NOT_HANDLED;

		break;
	}

	return res;
}

static int intc_priv_add(struct __device *dev)
{
	struct intc_priv_priv *priv = dev->priv;
	struct k_intc_device *intc = k_intc_from_dev(dev);
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->intc = intc;

	r = k_cpu_get_cpu_from_config(dev, 0, &priv->cpu_parent);
	if (r) {
		return r;
	}

	priv->hnd_ex.func  = intc_priv_intr;
	priv->hnd_ex.priv  = priv;
	priv->hnd_tm.func  = intc_priv_intr;
	priv->hnd_tm.priv  = priv;
	priv->hnd_sw.func  = intc_priv_intr;
	priv->hnd_sw.priv  = priv;

	r = k_arch_set_intr_handler(RV_CAUSE_INT_EX, &priv->hnd_ex);
	if (r) {
		return r;
	}

	r = k_arch_set_intr_handler(RV_CAUSE_INT_TM, &priv->hnd_tm);
	if (r) {
		return r;
	}

	r = k_arch_set_intr_handler(RV_CAUSE_INT_SW, &priv->hnd_sw);
	if (r) {
		return r;
	}

	/*
	 * Need to add hook function for enabling sw, timer and external
	 * interrupt (setup mie CSR) for each CPU because RISC-V specification
	 * do not allow to set mie CSR of other harts.
	 */
	priv->hnd_wake.func  = intc_priv_cpu_event;
	priv->hnd_wake.priv  = priv;
	priv->hnd_sleep.func  = intc_priv_cpu_event;
	priv->hnd_sleep.priv  = priv;

	r = k_cpu_add_handler(priv->cpu_parent, CPU_EVENT_ON_WAKEUP, &priv->hnd_wake);
	if (r) {
		return r;
	}

	r = k_cpu_add_handler(priv->cpu_parent, CPU_EVENT_ON_SLEEP, &priv->hnd_sleep);
	if (r) {
		return r;
	}

	priv->enabled_ex = 1;
	priv->enabled_tm = 1;
	priv->enabled_sw = 1;

	return 0;
}

static int intc_priv_remove(struct __device *dev)
{
	struct intc_priv_priv *priv = dev->priv;
	int r;

	r = k_cpu_remove_handler(priv->cpu_parent, CPU_EVENT_ON_WAKEUP, &priv->hnd_wake);
	if (r) {
		return r;
	}

	r = k_cpu_remove_handler(priv->cpu_parent, CPU_EVENT_ON_SLEEP, &priv->hnd_sleep);
	if (r) {
		return r;
	}

	return -ENOTSUP;
}

static int intc_priv_add_handler(struct k_intc_device *intc, int event, struct __event_handler *handler)
{
	struct __device *dev = k_intc_to_dev(intc);
	struct intc_priv_priv *priv = dev->priv;
	struct __event_handler *head;
	int r;

	switch (event) {
	case RV_IX_USIX:
	case RV_IX_SSIX:
	case RV_IX_MSIX:
		head = &priv->hnd_sw;
		break;
	case RV_IX_UTIX:
	case RV_IX_STIX:
	case RV_IX_MTIX:
		head = &priv->hnd_tm;
		break;
	case RV_IX_UEIX:
	case RV_IX_SEIX:
	case RV_IX_MEIX:
		head = &priv->hnd_ex;
		break;
	default:
		__dev_err(dev, "Unknown event number %d\n", event);
		return -EINVAL;
	}

	r = __event_add_handler(head, handler);
	if (r) {
		return r;
	}

	priv->has_ex = __event_has_next(&priv->hnd_ex);
	priv->has_tm = __event_has_next(&priv->hnd_tm);
	priv->has_sw = __event_has_next(&priv->hnd_sw);

	return 0;
}

static int intc_priv_remove_handler(struct k_intc_device *intc, int event, struct __event_handler *handler)
{
	struct __device *dev = k_intc_to_dev(intc);
	struct intc_priv_priv *priv = dev->priv;
	struct __event_handler *head;
	int r;

	switch (event) {
	case RV_IX_USIX:
	case RV_IX_SSIX:
	case RV_IX_MSIX:
		head = &priv->hnd_sw;
		break;
	case RV_IX_UTIX:
	case RV_IX_STIX:
	case RV_IX_MTIX:
		head = &priv->hnd_tm;
		break;
	case RV_IX_UEIX:
	case RV_IX_SEIX:
	case RV_IX_MEIX:
		head = &priv->hnd_ex;
		break;
	default:
		__dev_err(dev, "Unknown event number %d\n", event);
		return -EINVAL;
	}

	r = __event_remove_handler(head, handler);
	if (r) {
		return r;
	}

	priv->has_ex = __event_has_next(&priv->hnd_ex);
	priv->has_tm = __event_has_next(&priv->hnd_tm);
	priv->has_sw = __event_has_next(&priv->hnd_sw);

	return 0;
}

static int intc_priv_enable(struct k_intc_device *intc, int event)
{
	struct __device *dev = k_intc_to_dev(intc);
	struct intc_priv_priv *priv = dev->priv;

	switch (event) {
	case RV_IX_USIX:
	case RV_IX_SSIX:
	case RV_IX_MSIX:
		/* Cannot enable/disable SW interrupt */
		break;
	case RV_IX_UTIX:
	case RV_IX_STIX:
	case RV_IX_MTIX:
		priv->enabled_tm = 1;
		break;
	case RV_IX_UEIX:
	case RV_IX_SEIX:
	case RV_IX_MEIX:
		priv->enabled_ex = 1;
		break;
	default:
		__dev_err(dev, "Unknown event number %d\n", event);
		return -EINVAL;
	}

	/* FIXME: Need IPI event */

	return 0;
}

static int intc_priv_disable(struct k_intc_device *intc, int event)
{
	struct __device *dev = k_intc_to_dev(intc);
	struct intc_priv_priv *priv = dev->priv;

	switch (event) {
	case RV_IX_USIX:
	case RV_IX_SSIX:
	case RV_IX_MSIX:
		/* Cannot enable/disable SW interrupt */
		break;
	case RV_IX_UTIX:
	case RV_IX_STIX:
	case RV_IX_MTIX:
		priv->enabled_tm = 0;
		break;
	case RV_IX_UEIX:
	case RV_IX_SEIX:
	case RV_IX_MEIX:
		priv->enabled_ex = 0;
		break;
	default:
		__dev_err(dev, "Unknown event number %d\n", event);
		return -EINVAL;
	}

	/* FIXME: Need IPI event */

	return 0;
}

const static struct __device_driver_ops intc_priv_dev_ops = {
	.add = intc_priv_add,
	.remove = intc_priv_remove,
	.mmap = __device_driver_mmap,
};

const static struct k_intc_driver_ops intc_priv_intc_ops = {
	.add_handler = intc_priv_add_handler,
	.remove_handler = intc_priv_remove_handler,
	.enable = intc_priv_enable,
	.disable = intc_priv_disable,
};

static struct k_intc_driver intc_priv_drv = {
	.base = {
		.base = {
			.type_vendor = "riscv",
			.type_device = "priv1_10",
		},

		.ops = &intc_priv_dev_ops,
	},

	.ops = &intc_priv_intc_ops,
};

static int intc_priv_init(void)
{
	k_intc_add_driver(&intc_priv_drv);

	return 0;
}
define_init_func(intc_priv_init);
