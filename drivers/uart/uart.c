/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/uart.h>

static struct __uart_device *uart_default;

static int uart_putc(int c)
{
	if (!uart_default) {
		return (unsigned char)c;
	}

	const struct __uart_driver *drv = __get_uart_drv(__to_dev(uart_default));

	if (drv && drv->char_out) {
		drv->char_out(uart_default, c);
	}

	return (unsigned char)c;
}

static struct __uart_device *uart_get_default_console(void)
{
	return uart_default;
}

static int uart_set_default_console(struct __uart_device *uart)
{
	uart_default = uart;

	__set_printk_out(uart_putc);

	return 0;
}

int __uart_add_device(struct __uart_device *uart, struct __bus *parent, int set_default)
{
	int r;

	r = __device_add(__to_dev(uart), parent);
	if (r) {
		return r;
	}

	if (set_default) {
		r = uart_set_default_console(uart);
		if (r) {
			return r;
		}
	}

	return 0;
}

int __uart_remove_device(struct __uart_device *uart)
{
	if (uart_get_default_console() == uart) {
		uart_set_default_console(NULL);
	}

	return __device_remove(__to_dev(uart));
}

