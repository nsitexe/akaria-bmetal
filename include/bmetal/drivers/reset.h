/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_RESET_H_
#define BAREMETAL_CRT_DRIVERS_RESET_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct k_reset_config {
	uint32_t baud;
};

struct k_reset_device;

struct k_reset_driver_ops {
	int (*reboot)(struct k_reset_device *reset);
	int (*shutdown)(struct k_reset_device *reset);
	int (*power_off)(struct k_reset_device *reset);
};

struct k_reset_driver {
	struct __device_driver base;

	const struct k_reset_driver_ops *ops;
};

struct k_reset_device {
	struct __device base;

	int as_default;
};

struct k_reset_priv_max {
	char dummy[80];
};
typedef struct k_reset_priv_max    k_reset_priv_t;
#define CHECK_PRIV_SIZE_RESET(typ)    CHECK_PRIV_SIZE(typ, k_reset_priv_t);

static inline const struct k_reset_driver *k_reset_get_drv(const struct k_reset_device *reset)
{
	if (!reset) {
		return NULL;
	}

	return (const struct k_reset_driver *)reset->base.drv;
}

static inline struct __device *k_reset_to_dev(struct k_reset_device *reset)
{
	if (!reset) {
		return NULL;
	}

	return &reset->base;
}

static inline struct k_reset_device *k_reset_from_dev(struct __device *dev)
{
	return (struct k_reset_device *)dev;
}

static inline int k_reset_add_driver(struct k_reset_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int k_reset_remove_driver(struct k_reset_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

#ifdef CONFIG_RESET

struct k_reset_device *k_reset_get_system(void);
int k_reset_set_system(struct k_reset_device *reset);

int k_reset_add_device(struct k_reset_device *reset, struct __bus *parent);
int k_reset_remove_device(struct k_reset_device *reset);

#else /* CONFIG_RESET */

static inline struct k_reset_device *k_reset_get_system(void)
{
	return NULL;
}

static inline int k_reset_set_system(struct k_reset_device *reset)
{
	return -ENOTSUP;
}

static inline int k_reset_add_device(struct k_reset_device *reset, struct __bus *parent)
{
	return -ENOTSUP;
}

static inline int k_reset_remove_device(struct k_reset_device *reset)
{
	return -ENOTSUP;
}

#endif /* CONFIG_RESET */

#endif /* BAREMETAL_CRT_DRIVERS_RESET_H_ */
