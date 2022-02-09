/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_UART_H_
#define BAREMETAL_CRT_UART_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct __uart_device;

struct __uart_driver_ops {
	int (*char_in)(struct __uart_device *uart);
	void (*char_out)(struct __uart_device *uart, int value);
};

struct __uart_driver {
	struct __device_driver base;

	const struct __uart_driver_ops *ops;
};

struct __uart_device {
	struct __device base;

	int as_default;
};

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

int __uart_set_default_console(struct __uart_device *dev);
int __uart_add_device(struct __uart_device *dev, struct __bus *parent, int set_default);
int __uart_remove_device(struct __uart_device *dev);

#endif /* BAREMETAL_CRT_UART_H_ */
