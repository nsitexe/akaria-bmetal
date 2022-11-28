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
	struct __intc_device *intc;
	struct __cpu_device *cpu_parent;

	struct __event_handler hnd_ex;
	struct __event_handler hnd_tm;
	struct __event_handler hnd_sw;
	struct __event_handler hnd_cpu;
};
CHECK_PRIV_SIZE_INTC(struct intc_priv_priv);

static int intc_priv_intr_ex(int event, struct __event_handler *hnd)
{
	struct intc_priv_priv *priv = hnd->priv;

	return __event_handle_generic(event, priv->hnd_ex.hnd_next);
}

static int intc_priv_intr_tm(int event, struct __event_handler *hnd)
{
	struct intc_priv_priv *priv = hnd->priv;

	return __event_handle_generic(event, priv->hnd_tm.hnd_next);
}

static int intc_priv_intr_sw(int event, struct __event_handler *hnd)
{
	struct intc_priv_priv *priv = hnd->priv;

	return __event_handle_generic(event, priv->hnd_sw.hnd_next);
}

static int intc_priv_cpu_event(int event, struct __event_handler *hnd)
{
	/* TODO: implement timer interrupt */
	int m = XIX_SIX | XIX_EIX;

	switch (event) {
	case CPU_EVENT_ON_WAKEUP:
		__asm volatile ("csrs mie, %0" : : "r"(m));
		break;
	case CPU_EVENT_ON_SLEEP:
		__asm volatile ("csrc mie, %0" : : "r"(m));
		break;
	default:
		return EVENT_NOT_HANDLED;
	}

	return EVENT_HANDLED;
}

static int intc_priv_add(struct __device *dev)
{
	struct intc_priv_priv *priv = dev->priv;
	struct __intc_device *intc = __intc_from_dev(dev);
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->intc = intc;

	r = __cpu_get_cpu_from_config(dev, 0, &priv->cpu_parent);
	if (r) {
		return r;
	}

	priv->hnd_ex.func  = intc_priv_intr_ex;
	priv->hnd_ex.priv  = priv;
	priv->hnd_tm.func  = intc_priv_intr_tm;
	priv->hnd_tm.priv  = priv;
	priv->hnd_sw.func  = intc_priv_intr_sw;
	priv->hnd_sw.priv  = priv;

	r = __arch_set_intr_handler(RV_CAUSE_INT_EX, &priv->hnd_ex);
	if (r) {
		return r;
	}

	r = __arch_set_intr_handler(RV_CAUSE_INT_TM, &priv->hnd_tm);
	if (r) {
		return r;
	}

	r = __arch_set_intr_handler(RV_CAUSE_INT_SW, &priv->hnd_sw);
	if (r) {
		return r;
	}

	/*
	 * Need to add hook function for enabling sw, timer and external
	 * interrupt (setup mie CSR) for each CPU because RISC-V specification
	 * do not allow to set mie CSR of other harts.
	 */
	priv->hnd_cpu.func  = intc_priv_cpu_event;
	priv->hnd_cpu.priv  = priv;

	r = __cpu_add_handler(priv->cpu_parent, CPU_EVENT_ON_WAKEUP, &priv->hnd_cpu);
	if (r) {
		return r;
	}

	r = __cpu_add_handler(priv->cpu_parent, CPU_EVENT_ON_SLEEP, &priv->hnd_cpu);
	if (r) {
		return r;
	}

	return 0;
}

static int intc_priv_remove(struct __device *dev)
{
	struct intc_priv_priv *priv = dev->priv;
	int r;

	r = __cpu_remove_handler(priv->cpu_parent, CPU_EVENT_ON_WAKEUP, &priv->hnd_cpu);
	if (r) {
		return r;
	}

	r = __cpu_remove_handler(priv->cpu_parent, CPU_EVENT_ON_SLEEP, &priv->hnd_cpu);
	if (r) {
		return r;
	}

	return -ENOTSUP;
}

static int intc_priv_add_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	struct __device *dev = __intc_to_dev(intc);
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

	return 0;
}

static int intc_priv_remove_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	struct __device *dev = __intc_to_dev(intc);
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

	return 0;
}

const static struct __device_driver_ops intc_priv_dev_ops = {
	.add = intc_priv_add,
	.remove = intc_priv_remove,
	.mmap = __device_driver_mmap,
};

const static struct __intc_driver_ops intc_priv_intc_ops = {
	.add_handler = intc_priv_add_handler,
	.remove_handler = intc_priv_remove_handler,
};

static struct __intc_driver intc_priv_drv = {
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
	__intc_add_driver(&intc_priv_drv);

	return 0;
}
define_init_func(intc_priv_init);
