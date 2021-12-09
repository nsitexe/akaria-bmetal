/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_UART_H_
#define BAREMETAL_CRT_UART_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct __uart_device;

struct __uart_driver {
	struct __device_driver base;

	int (*char_in)(struct __uart_device *uart);
	void (*char_out)(struct __uart_device *uart, int value);
};

struct __uart_device {
	struct __device base;

	int as_default;
};

static inline const struct __uart_driver *__get_uart_drv(const struct __device *dev)
{
	if (!dev) {
		return NULL;
	}

	return (const struct __uart_driver *)dev->drv;
}

static inline struct __device *__to_dev(struct __uart_device *dev)
{
	if (!dev) {
		return NULL;
	}

	return &dev->base;
}

static inline struct __uart_device *__to_uart(struct __device *dev)
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
