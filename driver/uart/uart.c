/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/uart.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/stdio.h>
#include <bmetal/sys/string.h>

static struct __uart_device *uart_default;

int __uart_get_config(struct __uart_device *uart, struct __uart_config *conf)
{
	const struct __uart_driver *drv = __uart_get_drv(uart);
	int r;

	if (!uart) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->get_config) {
		r = drv->ops->get_config(uart, conf);
		if (r) {
			__dev_err(__uart_to_dev(uart), "failed to get UART config.\n");
			return r;
		}
	}

	return 0;
}

int __uart_set_config(struct __uart_device *uart, const struct __uart_config *conf)
{
	const struct __uart_driver *drv = __uart_get_drv(uart);
	int r;

	if (!uart) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->set_config) {
		r = drv->ops->set_config(uart, conf);
		if (r) {
			__dev_err(__uart_to_dev(uart), "failed to set UART config.\n");
			return r;
		}
	}

	return 0;
}

static int uart_getc(void)
{
	struct __device *uart_default_dev = __uart_to_dev(uart_default);
	int c = EOF;

	if (!uart_default || !uart_default_dev->probed) {
		return EOF;
	}

	const struct __uart_driver *drv = __uart_get_drv(uart_default);

	if (drv && drv->ops && drv->ops->char_in) {
		c = drv->ops->char_in(uart_default);
	}

	return c;
}

static int uart_putc(int c)
{
	struct __device *uart_default_dev = __uart_to_dev(uart_default);

	if (!uart_default || !uart_default_dev->probed) {
		return (unsigned char)c;
	}

	const struct __uart_driver *drv = __uart_get_drv(uart_default);

	if (drv && drv->ops && drv->ops->char_out) {
		drv->ops->char_out(uart_default, c);
	}

	return (unsigned char)c;
}

static struct __uart_device *uart_get_default_console(void)
{
	return uart_default;
}

int __uart_set_default_console(struct __uart_device *uart)
{
	uart_default = uart;

	__set_printk_in(uart_getc);
	__set_printk_out(uart_putc);

	return 0;
}

int __uart_read_default_config(struct __uart_device *uart, struct __uart_config *conf)
{
	struct __device *dev = __uart_to_dev(uart);

	kmemset(conf, 0, sizeof(*conf));

	__device_read_conf_u32(dev, "baud", &conf->baud, 0);

	return 0;
}

int __uart_add_device(struct __uart_device *uart, struct __bus *parent, int set_default)
{
	int r;

	r = __device_add(__uart_to_dev(uart), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	if (set_default) {
		r = __uart_set_default_console(uart);
		if (r) {
			return r;
		}
	}

	return 0;
}

int __uart_remove_device(struct __uart_device *uart)
{
	if (uart_get_default_console() == uart) {
		__uart_set_default_console(NULL);
	}

	return __device_remove(__uart_to_dev(uart));
}
