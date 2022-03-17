/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>

#include <bmetal/drivers/intc.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>

int __intc_add_device(struct __intc_device *intc, struct __bus *parent)
{
	int r;

	r = __device_add(__intc_to_dev(intc), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	return 0;
}

int __intc_remove_device(struct __intc_device *intc)
{
	return __device_remove(__intc_to_dev(intc));
}

int __intc_add_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	const struct __intc_driver *drv = __intc_get_drv(intc);
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

int __intc_remove_handler(struct __intc_device *intc, int event, struct __event_handler *handler)
{
	const struct __intc_driver *drv = __intc_get_drv(intc);
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

int __intc_handle_generic_event(struct __intc_device *intc, int event, struct __event_handler *hnd_head)
{
	int r, res = EVENT_NOT_HANDLED;

	for_each_handler (h, hnd_head) {
		/* h->func is not NULL because checked at add function */
		if (h->event == event) {
			r = h->func(event, h);
			if (r == EVENT_HANDLED) {
				res = EVENT_HANDLED;
			}
		}
	}

	return res;
}

int __intc_get_conf_length(struct __device *dev, int *len)
{
	int v, r;

	r = __device_get_conf_length(dev, "interrupts", &v);
	if (r) {
		return r;
	}

	/* interrupts consists from "intc parent", "irq number" */
	if (len) {
		*len = v / 2;
	}

	return 0;
}

int __intc_get_intc_from_config(struct __device *dev, int index, struct __intc_device **intc, int *num_irq)
{
	const char *intc_name;
	struct __device *tmp;
	uint32_t val;
	int r;

	r = __device_read_conf_str(dev, "interrupts", &intc_name, index * 2);
	if (r) {
		__dev_err(dev, "intc name is not found, index:%d.\n", index);
		return -EINVAL;
	}
	r = __device_read_conf_u32(dev, "interrupts", &val, index * 2 + 1);
	if (r) {
		__dev_err(dev, "intc irq number is not found, index:%d.\n", index);
		return -EINVAL;
	}

	r = __bus_find_device(__bus_get_root(), intc_name, &tmp);
	if (r == -EAGAIN) {
		return -EAGAIN;
	} else if (r) {
		__dev_err(dev, "intc '%s' is not found.\n", intc_name);
		return -EINVAL;
	}

	if (intc) {
		*intc = __intc_from_dev(tmp);
	}
	if (num_irq) {
		*num_irq = val;
	}

	return 0;
}
