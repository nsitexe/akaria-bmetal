/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/driver/intc.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/driver/cpu.h>
#include <bmetal/sys/errno.h>

static struct k_intc_device *intc_ipi;

static struct k_intc_device *intc_get_ipi(void)
{
	return intc_ipi;
}

int k_intc_set_ipi(struct k_intc_device *intc)
{
	intc_ipi = intc;

	return 0;
}

int k_intc_raise_ipi(struct k_cpu_device *src, struct k_cpu_device *dest, void *arg)
{
	struct k_intc_device *intc = intc_get_ipi();
	const struct k_intc_driver *drv = k_intc_get_drv(intc);
	int r;

	if (!intc) {
		__dev_err(k_cpu_to_dev(src), "not set IPI intc.\n");
		return -ENOTSUP;
	}

	if (drv && drv->ops && drv->ops->raise_ipi) {
		r = drv->ops->raise_ipi(intc, src, dest, arg);
		if (r) {
			__dev_err(k_intc_to_dev(intc), "failed to raise IPI.\n");
			return r;
		}
	} else {
		__dev_err(k_intc_to_dev(intc), "not supported to raise IPI.\n");
		return -ENOTSUP;
	}

	return 0;
}

int k_intc_add_device(struct k_intc_device *intc, struct k_bus *parent)
{
	struct k_device *dev = k_intc_to_dev(intc);
	uint32_t val;
	int r;

	r = k_device_add(dev, parent);
	if (IS_ERROR(r)) {
		return r;
	}

	r = k_device_read_conf_u32(dev, "ipi", &val, 0);
	if (r) {
		/* Not found, default value is 0 */
		val = 0;
	}
	if (val) {
		k_intc_set_ipi(intc);
	}

	return 0;
}

int k_intc_remove_device(struct k_intc_device *intc)
{
	return k_device_remove(k_intc_to_dev(intc));
}

int k_intc_add_handler(struct k_intc_device *intc, int event, struct k_event_handler *handler)
{
	const struct k_intc_driver *drv = k_intc_get_drv(intc);
	int r = -ENODEV;

	if (!handler || !handler->func) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->add_handler) {
		r = drv->ops->add_handler(intc, event, handler);
		if (!r) {
			handler->event = event;
		}
	}

	return r;
}

int k_intc_remove_handler(struct k_intc_device *intc, int event, struct k_event_handler *handler)
{
	const struct k_intc_driver *drv = k_intc_get_drv(intc);
	int r = -ENODEV;

	if (!handler || !handler->func) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->remove_handler) {
		r = drv->ops->remove_handler(intc, event, handler);
		if (!r) {
			handler->hnd_next = NULL;
		}
	}

	return r;
}

int k_intc_get_conf_length(struct k_device *dev, int *len)
{
	int v, r;

	r = k_device_get_conf_length(dev, "interrupts", &v);
	if (r) {
		return r;
	}

	/* interrupts consists from "intc parent", "irq number" */
	if (len) {
		*len = v / 2;
	}

	return 0;
}

int k_intc_get_intc_from_config(struct k_device *dev, int index, struct k_intc_device **intc, int *num_irq)
{
	const char *intc_name;
	struct k_device *tmp;
	uint32_t val;
	int r;

	r = k_device_read_conf_str(dev, "interrupts", &intc_name, index * 2);
	if (r) {
		__dev_err(dev, "intc name is not found, index:%d.\n", index);
		return -EINVAL;
	}
	r = k_device_read_conf_u32(dev, "interrupts", &val, index * 2 + 1);
	if (r) {
		__dev_err(dev, "intc irq number is not found, index:%d.\n", index);
		return -EINVAL;
	}

	r = k_bus_find_device(k_bus_get_root(), intc_name, &tmp);
	if (r == -EAGAIN) {
		return -EAGAIN;
	} else if (r) {
		__dev_err(dev, "intc '%s' is not found.\n", intc_name);
		return -EINVAL;
	}

	if (intc) {
		*intc = k_intc_from_dev(tmp);
	}
	if (num_irq) {
		*num_irq = val;
	}

	return 0;
}
