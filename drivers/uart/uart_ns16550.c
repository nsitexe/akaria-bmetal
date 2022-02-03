/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <inttypes.h>

#include <bmetal/drivers/uart.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>

#define REG_RDR     0x00  /* Receiver data       */
#define REG_THR     0x00  /* Transmitter holding */
#define REG_IER     0x01  /* Interrupt enable    */
#define REG_IIR     0x02  /* Interrupt ID        */
#define REG_FCR     0x02  /* FIFO control        */
#define REG_LCR     0x03  /* Line control        */
#define REG_MDC     0x04  /* Modem control       */
#define REG_LSR     0x05  /* Line status         */
#define REG_MSR     0x06  /* Modem status        */
#define REG_SCR     0x07  /* Scratch pad         */
#define REG_BRDL    0x00  /* Divisor latch (LSB) */
#define REG_BRDH    0x01  /* Divisor latch (MSB) */

#define REG_WIDTH_DEFAULT    4

static uint32_t addr_shift;

static uint8_t uart_read(struct __uart_device *uart, uintptr_t off)
{
	uintptr_t shifted = off << addr_shift;

	switch (addr_shift) {
	case 0:
		return __device_read8(__uart_to_dev(uart), shifted);
	case 1:
		return __device_read16(__uart_to_dev(uart), shifted);
	case 2:
		return __device_read32(__uart_to_dev(uart), shifted);
	default:
		/* BUG */
		return -1;
	}
}

static void uart_write(struct __uart_device *uart, uint8_t dat, uintptr_t off)
{
	uintptr_t shifted = off << addr_shift;

	switch (addr_shift) {
	case 0:
		__device_write8(__uart_to_dev(uart), dat, shifted);
		break;
	case 1:
		__device_write16(__uart_to_dev(uart), dat, shifted);
		break;
	case 2:
		__device_write32(__uart_to_dev(uart), dat, shifted);
		break;
	default:
		/* BUG */
		return;
	}
}

static int uart_ns16550_add(struct __device *dev)
{
	uint32_t w;
	int r;

	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	r = __device_read_conf_u32(dev, "reg-width", &w);
	if (r) {
		w = REG_WIDTH_DEFAULT;

		printk("ns16550: config 'reg-width' is not found."
			"Use default size %"PRId32".\n", w);
	}
	switch (w) {
	case 1:
		addr_shift = 0;
		break;
	case 2:
		addr_shift = 1;
		break;
	case 4:
		addr_shift = 2;
		break;
	default:
		printk("ns16550: config 'reg-width' %"PRId32" is not supported.\n", w);
		return -EINVAL;
	}

	return 0;
}

static int uart_ns16550_remove(struct __device *dev)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}

int uart_ns16550_char_in(struct __uart_device *uart)
{
	return 0;
}

void uart_ns16550_char_out(struct __uart_device *uart, int value)
{
	while ((uart_read(uart, REG_LSR) & 0x20) == 0) {
	}

	uart_write(uart, value, REG_THR);
}

static struct __uart_driver ns16550_drv = {
	.base = {
		.base = {
			.type_vendor = "ns",
			.type_device = "ns16550",
		},

		.add = uart_ns16550_add,
		.remove = uart_ns16550_remove,
		.mmap = __device_driver_mmap,
	},

	.char_in = uart_ns16550_char_in,
	.char_out = uart_ns16550_char_out,
};

static int uart_ns16550_init(void)
{
	__uart_add_driver(&ns16550_drv);

	return 0;
}

define_init_func(uart_ns16550_init);
