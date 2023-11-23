/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_INTC_H_
#define BAREMETAL_CRT_DRIVERS_INTC_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>
#include <bmetal/event.h>
#include <bmetal/intr.h>
#include <bmetal/sys/errno.h>

struct k_cpu_device;
struct k_intc_device;

struct k_intc_driver_ops {
	int (*add_handler)(struct k_intc_device *intc, int event, struct __event_handler *handler);
	int (*remove_handler)(struct k_intc_device *intc, int event, struct __event_handler *handler);
	int (*enable)(struct k_intc_device *intc, int event);
	int (*disable)(struct k_intc_device *intc, int event);
	int (*raise_ipi)(struct k_intc_device *intc, struct k_cpu_device *src, struct k_cpu_device *dest, void *arg);
};

struct k_intc_driver {
	struct __device_driver base;

	const struct k_intc_driver_ops *ops;
};

struct k_intc_device {
	struct __device base;
};

struct k_intc_priv_max {
	char dummy[560];
};
typedef struct k_intc_priv_max    k_intc_priv_t;
#define CHECK_PRIV_SIZE_INTC(typ)    CHECK_PRIV_SIZE(typ, k_intc_priv_t);

static inline const struct k_intc_driver *k_intc_get_drv(const struct k_intc_device *intc)
{
	if (!intc) {
		return NULL;
	}

	return (const struct k_intc_driver *)intc->base.drv;
}

static inline struct __device *k_intc_to_dev(struct k_intc_device *intc)
{
	if (!intc) {
		return NULL;
	}

	return &intc->base;
}

static inline struct k_intc_device *k_intc_from_dev(struct __device *dev)
{
	return (struct k_intc_device *)dev;
}

static inline int k_intc_add_driver(struct k_intc_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int k_intc_remove_driver(struct k_intc_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

#ifdef CONFIG_INTC

int k_intc_set_ipi(struct k_intc_device *intc);
int k_intc_raise_ipi(struct k_cpu_device *src, struct k_cpu_device *dest, void *arg);

int k_intc_add_device(struct k_intc_device *intc, struct __bus *parent);
int k_intc_remove_device(struct k_intc_device *intc);
int k_intc_add_handler(struct k_intc_device *intc, int event, struct __event_handler *handler);
int k_intc_remove_handler(struct k_intc_device *intc, int event, struct __event_handler *handler);

int k_intc_get_conf_length(struct __device *dev, int *len);
int k_intc_get_intc_from_config(struct __device *dev, int index, struct k_intc_device **intc, int *num_irq);

#else /* CONFIG_INTC */

static inline int k_intc_add_device(struct k_intc_device *intc, struct __bus *parent)
{
	return -ENOTSUP;
}

static inline int k_intc_remove_device(struct k_intc_device *intc)
{
	return -ENOTSUP;
}

static inline int k_intc_add_handler(struct k_intc_device *intc, int event, struct __event_handler *handler)
{
	return -ENOTSUP;
}

static inline int k_intc_remove_handler(struct k_intc_device *intc, int event, struct __event_handler *handler)
{
	return -ENOTSUP;
}

static inline int k_intc_handle_generic_event(struct k_intc_device *intc, int event, struct __event_handler *hnd_head)
{
	return -ENOTSUP;
}


static inline int k_intc_get_conf_length(struct __device *dev, int *len)
{
	return -ENOTSUP;
}

static inline int k_intc_get_intc_from_config(struct __device *dev, int index, struct k_intc_device **intc, int *num_irq)
{
	return -ENOTSUP;
}

#endif /* CONFIG_INTC */

#endif /* BAREMETAL_CRT_DRIVERS_INTC_H_ */
