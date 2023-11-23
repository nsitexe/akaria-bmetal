/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_CLK_H_
#define BAREMETAL_CRT_DRIVERS_CLK_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

#define KHZ    (1000)
#define MHZ    (1000000)
#define GHZ    (1000000000)

struct k_clk_device;

struct k_clk_driver_ops {
	int (* enable)(struct k_clk_device *clk, int index);
	int (* disable)(struct k_clk_device *clk, int index);
	int (* get_freq)(struct k_clk_device *clk, int index, uint64_t *freq);
	int (* set_freq)(struct k_clk_device *clk, int index, uint64_t freq);
};

struct k_clk_driver {
	struct k_device_driver base;

	const struct k_clk_driver_ops *ops;
};

struct k_clk_device {
	struct k_device base;
};

struct k_clk_priv_max {
	char dummy[56];
};
typedef struct k_clk_priv_max    k_clk_priv_t;
#define CHECK_PRIV_SIZE_CLK(typ)    CHECK_PRIV_SIZE(typ, k_clk_priv_t);

static inline const struct k_clk_driver *k_clk_get_drv(const struct k_clk_device *clk)
{
	if (!clk) {
		return NULL;
	}

	return (const struct k_clk_driver *)clk->base.drv;
}

static inline struct k_device *k_clk_to_dev(struct k_clk_device *clk)
{
	if (!clk) {
		return NULL;
	}

	return &clk->base;
}

static inline struct k_clk_device *k_clk_from_dev(struct k_device *dev)
{
	return (struct k_clk_device *)dev;
}

static inline int k_clk_add_driver(struct k_clk_driver *drv)
{
	return k_driver_add(&drv->base.base);
}

static inline int k_clk_remove_driver(struct k_clk_driver *drv)
{
	return k_driver_remove(&drv->base.base);
}

#ifdef CONFIG_CLK

int k_clk_add_device(struct k_clk_device *clk, struct k_bus *parent);
int k_clk_remove_device(struct k_clk_device *clk);
int k_clk_enable(struct k_clk_device *clk, int index);
int k_clk_disable(struct k_clk_device *clk, int index);
int k_clk_get_frequency(struct k_clk_device *clk, int index, uint64_t *freq);
int k_clk_set_frequency(struct k_clk_device *clk, int index, uint64_t freq);

int k_clk_get_clk_from_config(struct k_device *dev, int index, struct k_clk_device **clk, int *clk_index);

#else /* CONFIG_CLK */

static inline int k_clk_add_device(struct k_clk_device *clk, struct k_bus *parent)
{
	return -ENOTSUP;
}

static inline int k_clk_remove_device(struct k_clk_device *clk)
{
	return -ENOTSUP;
}

static inline int k_clk_enable(struct k_clk_device *clk, int index)
{
	return -ENOTSUP;
}

static inline int k_clk_disable(struct k_clk_device *clk, int index)
{
	return -ENOTSUP;
}

static inline int k_clk_get_frequency(struct k_clk_device *clk, int index, uint64_t *freq)
{
	return -ENOTSUP;
}

static inline int k_clk_set_frequency(struct k_clk_device *clk, int index, uint64_t freq)
{
	return -ENOTSUP;
}

static inline int k_clk_get_clk_from_config(struct k_device *dev, int index, struct k_clk_device **clk, int *clk_index)
{
	return -ENOTSUP;
}

#endif /* CONFIG_CLK */

#endif /* BAREMETAL_CRT_DRIVERS_CLK_H_ */
