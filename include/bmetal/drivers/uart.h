/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_UART_H_
#define BAREMETAL_CRT_DRIVERS_UART_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>

struct k_uart_config {
	uint32_t baud;
};

struct k_uart_device;

struct k_uart_driver_ops {
	int (*char_in)(struct k_uart_device *uart);
	void (*char_out)(struct k_uart_device *uart, int value);
	int (*get_config)(struct k_uart_device *uart, struct k_uart_config *conf);
	int (*set_config)(struct k_uart_device *uart, const struct k_uart_config *conf);
};

struct k_uart_driver {
	struct k_device_driver base;

	const struct k_uart_driver_ops *ops;
};

struct k_uart_device {
	struct k_device base;

	int as_default;
};

struct k_uart_priv_max {
	char dummy[88];
};
typedef struct k_uart_priv_max    k_uart_priv_t;
#define CHECK_PRIV_SIZE_UART(typ)    CHECK_PRIV_SIZE(typ, k_uart_priv_t);

static inline const struct k_uart_driver *k_uart_get_drv(const struct k_uart_device *uart)
{
	if (!uart) {
		return NULL;
	}

	return (const struct k_uart_driver *)uart->base.drv;
}

static inline struct k_device *k_uart_to_dev(struct k_uart_device *uart)
{
	if (!uart) {
		return NULL;
	}

	return &uart->base;
}

static inline struct k_uart_device *k_uart_from_dev(struct k_device *dev)
{
	return (struct k_uart_device *)dev;
}

static inline int k_uart_add_driver(struct k_uart_driver *drv)
{
	return k_driver_add(&drv->base.base);
}

static inline int k_uart_remove_driver(struct k_uart_driver *drv)
{
	return k_driver_remove(&drv->base.base);
}

#ifdef CONFIG_UART

int k_uart_get_config(struct k_uart_device *uart, struct k_uart_config *conf);
int k_uart_set_config(struct k_uart_device *uart, const struct k_uart_config *conf);
int k_uart_set_default_console(struct k_uart_device *uart);
int k_uart_read_default_config(struct k_uart_device *uart, struct k_uart_config *conf);

int k_uart_add_device(struct k_uart_device *uart, struct k_bus *parent, int set_default);
int k_uart_remove_device(struct k_uart_device *uart);

#else /* CONFIG_UART */

static inline int k_uart_get_config(struct k_uart_device *uart, struct k_uart_config *conf)
{
	return -ENOTSUP;
}

static inline int k_uart_set_config(struct k_uart_device *uart, const struct k_uart_config *conf)
{
	return -ENOTSUP;
}

static inline int k_uart_set_default_console(struct k_uart_device *uart)
{
	return -ENOTSUP;
}

static inline int k_uart_read_default_config(struct k_uart_device *uart, struct k_uart_config *conf)
{
	return -ENOTSUP;
}


static inline int k_uart_add_device(struct k_uart_device *uart, struct k_bus *parent, int set_default)
{
	return -ENOTSUP;
}

static inline int k_uart_remove_device(struct k_uart_device *uart)
{
	return -ENOTSUP;
}

#endif /* CONFIG_UART */

#endif /* BAREMETAL_CRT_DRIVERS_UART_H_ */
