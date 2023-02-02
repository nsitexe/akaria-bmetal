/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_RESET_H_
#define BAREMETAL_CRT_DRIVERS_RESET_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct __reset_config {
	uint32_t baud;
};

struct __reset_device;

struct __reset_driver_ops {
	int (*reboot)(struct __reset_device *reset);
	int (*shutdown)(struct __reset_device *reset);
	int (*power_off)(struct __reset_device *reset);
};

struct __reset_driver {
	struct __device_driver base;

	const struct __reset_driver_ops *ops;
};

struct __reset_device {
	struct __device base;

	int as_default;
};

struct __reset_priv_max {
	char dummy[80];
};
typedef struct __reset_priv_max    __reset_priv_t;
#define CHECK_PRIV_SIZE_RESET(typ)    CHECK_PRIV_SIZE(typ, __reset_priv_t);

static inline const struct __reset_driver *__reset_get_drv(const struct __reset_device *reset)
{
	if (!reset) {
		return NULL;
	}

	return (const struct __reset_driver *)reset->base.drv;
}

static inline struct __device *__reset_to_dev(struct __reset_device *reset)
{
	if (!reset) {
		return NULL;
	}

	return &reset->base;
}

static inline struct __reset_device *__reset_from_dev(struct __device *dev)
{
	return (struct __reset_device *)dev;
}

static inline int __reset_add_driver(struct __reset_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int __reset_remove_driver(struct __reset_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

#ifdef CONFIG_RESET

struct __reset_device *__reset_get_system(void);
int __reset_set_system(struct __reset_device *reset);

int __reset_add_device(struct __reset_device *reset, struct __bus *parent);
int __reset_remove_device(struct __reset_device *reset);

#else /* CONFIG_RESET */

static inline struct __reset_device *__reset_get_system(void)
{
	return NULL;
}

static inline int __reset_set_system(struct __reset_device *reset)
{
	return -ENOTSUP;
}

static inline int __reset_add_device(struct __reset_device *reset, struct __bus *parent)
{
	return -ENOTSUP;
}

static inline int __reset_remove_device(struct __reset_device *reset)
{
	return -ENOTSUP;
}

#endif /* CONFIG_RESET */

#endif /* BAREMETAL_CRT_DRIVERS_RESET_H_ */
