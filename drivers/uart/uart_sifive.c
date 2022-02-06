/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <inttypes.h>

#include <bmetal/drivers/uart.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>

#define REG_TXDATA     0x00  /* Transmit data */
#define REG_RXDATA     0x04  /* Receive data */
#define REG_TXCTRL     0x08  /* Transmit control */
#define REG_RXCTRL     0x0c  /* Receive control */
#define REG_IE         0x10  /* Interrupt enable */
#define REG_IP         0x14  /* Interrupt pending  */
#define REG_DIV        0x18  /* Baud rate divisor */

#define TXDATA_FULL    BIT(31)

static int uart_sifive_add(struct __device *dev)
{
	int r;

	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	return 0;
}

static int uart_sifive_remove(struct __device *dev)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}

int uart_sifive_char_in(struct __uart_device *uart)
{
	return 0;
}

void uart_sifive_char_out(struct __uart_device *uart, int value)
{
	struct __device *d = __uart_to_dev(uart);

	while ((__device_read32(d, REG_TXDATA) & TXDATA_FULL) != 0) {
	}

	__device_write32(d, value & 0xffU, REG_TXDATA);
}

static struct __uart_driver sifive_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "uart0",
		},

		.add = uart_sifive_add,
		.remove = uart_sifive_remove,
		.mmap = __device_driver_mmap,
	},

	.char_in = uart_sifive_char_in,
	.char_out = uart_sifive_char_out,
};

static int uart_sifive_init(void)
{
	__uart_add_driver(&sifive_drv);

	return 0;
}

define_init_func(uart_sifive_init);
