/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_CLK_H_
#define BAREMETAL_CRT_CLK_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct __clk_device;

struct __clk_driver_ops {
};

struct __clk_driver {
	struct __device_driver base;

	const struct __clk_driver_ops *ops;
};

struct __clk_device {
	struct __device base;

	int as_default;
};

static inline const struct __clk_driver *__clk_get_drv(const struct __clk_device *clk)
{
	if (!clk) {
		return NULL;
	}

	return (const struct __clk_driver *)clk->base.drv;
}

static inline struct __device *__clk_to_dev(struct __clk_device *clk)
{
	if (!clk) {
		return NULL;
	}

	return &clk->base;
}

static inline struct __clk_device *__clk_from_dev(struct __device *dev)
{
	return (struct __clk_device *)dev;
}

static inline int __clk_add_driver(struct __clk_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int __clk_remove_driver(struct __clk_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

int __clk_add_device(struct __clk_device *dev, struct __bus *parent);
int __clk_remove_device(struct __clk_device *dev);

#endif /* BAREMETAL_CRT_CLK_H_ */
