/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/uart.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/stdio.h>
#include <bmetal/sys/string.h>

static struct k_uart_device *uart_default;

int k_uart_get_config(struct k_uart_device *uart, struct k_uart_config *conf)
{
	const struct k_uart_driver *drv = k_uart_get_drv(uart);
	int r;

	if (!uart) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->get_config) {
		r = drv->ops->get_config(uart, conf);
		if (r) {
			__dev_err(k_uart_to_dev(uart), "failed to get UART config.\n");
			return r;
		}
	}

	return 0;
}

int k_uart_set_config(struct k_uart_device *uart, const struct k_uart_config *conf)
{
	const struct k_uart_driver *drv = k_uart_get_drv(uart);
	int r;

	if (!uart) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->set_config) {
		r = drv->ops->set_config(uart, conf);
		if (r) {
			__dev_err(k_uart_to_dev(uart), "failed to set UART config.\n");
			return r;
		}
	}

	return 0;
}

static int uart_getc(void)
{
	struct k_device *uart_default_dev = k_uart_to_dev(uart_default);
	int c = EOF;

	if (!uart_default || !uart_default_dev->probed) {
		return EOF;
	}

	const struct k_uart_driver *drv = k_uart_get_drv(uart_default);

	if (drv && drv->ops && drv->ops->char_in) {
		c = drv->ops->char_in(uart_default);
	}

	return c;
}

static int uart_putc(int c)
{
	struct k_device *uart_default_dev = k_uart_to_dev(uart_default);

	if (!uart_default || !uart_default_dev->probed) {
		return (unsigned char)c;
	}

	const struct k_uart_driver *drv = k_uart_get_drv(uart_default);

	if (drv && drv->ops && drv->ops->char_out) {
		drv->ops->char_out(uart_default, c);
	}

	return (unsigned char)c;
}

static struct k_uart_device *uart_get_default_console(void)
{
	return uart_default;
}

int k_uart_set_default_console(struct k_uart_device *uart)
{
	uart_default = uart;

	k_pri_set_getchar(uart_getc);
	k_pri_set_putchar(uart_putc);

	return 0;
}

int k_uart_read_default_config(struct k_uart_device *uart, struct k_uart_config *conf)
{
	struct k_device *dev = k_uart_to_dev(uart);

	k_memset(conf, 0, sizeof(*conf));

	k_device_read_conf_u32(dev, "baud", &conf->baud, 0);

	return 0;
}

int k_uart_add_device(struct k_uart_device *uart, struct k_bus *parent, int set_default)
{
	int r;

	r = k_device_add(k_uart_to_dev(uart), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	if (set_default) {
		r = k_uart_set_default_console(uart);
		if (r) {
			return r;
		}
	}

	return 0;
}

int k_uart_remove_device(struct k_uart_device *uart)
{
	if (uart_get_default_console() == uart) {
		k_uart_set_default_console(NULL);
	}

	return k_device_remove(k_uart_to_dev(uart));
}
