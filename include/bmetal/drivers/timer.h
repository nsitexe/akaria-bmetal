/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_TIMER_H_
#define BAREMETAL_CRT_DRIVERS_TIMER_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>
#include <bmetal/intr.h>
#include <bmetal/sys/errno.h>

struct __timer_device;

struct __timer_driver_ops {
	int (* enable)(struct __timer_device *tm, int index);
	int (* disable)(struct __timer_device *tm, int index);
	int (* get_freq)(struct __timer_device *tm, int index, uint64_t *freq);
	int (* set_freq)(struct __timer_device *tm, int index, uint64_t freq);
	int (* get_raw)(struct __timer_device *tm, int index, uint64_t *count);
	int (* set_raw)(struct __timer_device *tm, int index, uint64_t count);
};

struct __timer_driver {
	struct __device_driver base;

	const struct __timer_driver_ops *ops;
};

struct __timer_device {
	struct __device base;
};

struct __timer_priv_max {
	char dummy[144];
};
typedef struct __timer_priv_max    __timer_priv_t;
#define CHECK_PRIV_SIZE_TIMER(typ)    CHECK_PRIV_SIZE(typ, __timer_priv_t);

static inline const struct __timer_driver *__timer_get_drv(const struct __timer_device *timer)
{
	if (!timer) {
		return NULL;
	}

	return (const struct __timer_driver *)timer->base.drv;
}

static inline struct __device *__timer_to_dev(struct __timer_device *timer)
{
	if (!timer) {
		return NULL;
	}

	return &timer->base;
}

static inline struct __timer_device *__timer_from_dev(struct __device *dev)
{
	return (struct __timer_device *)dev;
}

static inline int __timer_add_driver(struct __timer_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int __timer_remove_driver(struct __timer_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

#ifdef CONFIG_TIMER

struct __timer_device *__timer_get_system(void);
int __timer_set_system(struct __timer_device *timer);

int __timer_add_device(struct __timer_device *timer, struct __bus *parent);
int __timer_remove_device(struct __timer_device *timer);

#else /* CONFIG_TIMER */

static inline struct __timer_device *__timer_get_system(void)
{
	return NULL;
}

static inline int __timer_set_system(struct __timer_device *timer)
{
	return -ENOTSUP;
}

static inline int __timer_add_device(struct __timer_device *timer, struct __bus *parent)
{
	return -ENOTSUP;
}

static inline int __timer_remove_device(struct __timer_device *timer)
{
	return -ENOTSUP;
}

#endif /* CONFIG_TIMER */

#endif /* BAREMETAL_CRT_DRIVERS_TIMER_H_ */
