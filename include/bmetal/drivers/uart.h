/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_UART_H_
#define BAREMETAL_CRT_DRIVERS_UART_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct __uart_config {
	uint32_t baud;
};

struct __uart_device;

struct __uart_driver_ops {
	int (*char_in)(struct __uart_device *uart);
	void (*char_out)(struct __uart_device *uart, int value);
	int (*get_config)(struct __uart_device *uart, struct __uart_config *conf);
	int (*set_config)(struct __uart_device *uart, const struct __uart_config *conf);
};

struct __uart_driver {
	struct __device_driver base;

	const struct __uart_driver_ops *ops;
};

struct __uart_device {
	struct __device base;

	int as_default;
};

struct __uart_priv_max {
	char dummy[88];
};
typedef struct __uart_priv_max    __uart_priv_t;
#define CHECK_PRIV_SIZE_UART(typ)    CHECK_PRIV_SIZE(typ, __uart_priv_t);

static inline const struct __uart_driver *__uart_get_drv(const struct __uart_device *uart)
{
	if (!uart) {
		return NULL;
	}

	return (const struct __uart_driver *)uart->base.drv;
}

static inline struct __device *__uart_to_dev(struct __uart_device *uart)
{
	if (!uart) {
		return NULL;
	}

	return &uart->base;
}

static inline struct __uart_device *__uart_from_dev(struct __device *dev)
{
	return (struct __uart_device *)dev;
}

static inline int __uart_add_driver(struct __uart_driver *drv)
{
	return __driver_add(&drv->base.base);
}

static inline int __uart_remove_driver(struct __uart_driver *drv)
{
	return __driver_remove(&drv->base.base);
}

#ifdef CONFIG_UART

int __uart_get_config(struct __uart_device *uart, struct __uart_config *conf);
int __uart_set_config(struct __uart_device *uart, const struct __uart_config *conf);
int __uart_set_default_console(struct __uart_device *uart);
int __uart_read_default_config(struct __uart_device *uart, struct __uart_config *conf);

int __uart_add_device(struct __uart_device *uart, struct __bus *parent, int set_default);
int __uart_remove_device(struct __uart_device *uart);

#else /* CONFIG_UART */

static inline int __uart_get_config(struct __uart_device *uart, struct __uart_config *conf)
{
	return -ENOTSUP;
}

static inline int __uart_set_config(struct __uart_device *uart, const struct __uart_config *conf)
{
	return -ENOTSUP;
}

static inline int __uart_set_default_console(struct __uart_device *uart)
{
	return -ENOTSUP;
}

static inline int __uart_read_default_config(struct __uart_device *uart, struct __uart_config *conf)
{
	return -ENOTSUP;
}


static inline int __uart_add_device(struct __uart_device *uart, struct __bus *parent, int set_default)
{
	return -ENOTSUP;
}

static inline int __uart_remove_device(struct __uart_device *uart)
{
	return -ENOTSUP;
}

#endif /* CONFIG_UART */

#endif /* BAREMETAL_CRT_DRIVERS_UART_H_ */
