/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_CLK_H_
#define BAREMETAL_CRT_DRIVERS_CLK_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

#define KHZ    (1000)
#define MHZ    (1000000)
#define GHZ    (1000000000)

struct __clk_device;

struct __clk_driver_ops {
	int (* enable)(struct __clk_device *clk, int index);
	int (* disable)(struct __clk_device *clk, int index);
	int (* get_freq)(struct __clk_device *clk, int index, uint64_t *freq);
	int (* set_freq)(struct __clk_device *clk, int index, uint64_t freq);
};

struct __clk_driver {
	struct __device_driver base;

	const struct __clk_driver_ops *ops;
};

struct __clk_device {
	struct __device base;
};

struct __clk_priv_max {
	char dummy[56];
};
typedef struct __clk_priv_max    __clk_priv_t;
#define CHECK_PRIV_SIZE_CLK(typ)    CHECK_PRIV_SIZE(typ, __clk_priv_t);

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

int __clk_add_device(struct __clk_device *clk, struct __bus *parent);
int __clk_remove_device(struct __clk_device *clk);
int __clk_enable(struct __clk_device *clk, int index);
int __clk_disable(struct __clk_device *clk, int index);
int __clk_get_frequency(struct __clk_device *clk, int index, uint64_t *freq);
int __clk_set_frequency(struct __clk_device *clk, int index, uint64_t freq);

int __clk_get_clk_from_config(struct __device *dev, int index, struct __clk_device **clk, int *clk_index);

#endif /* BAREMETAL_CRT_DRIVERS_CLK_H_ */
