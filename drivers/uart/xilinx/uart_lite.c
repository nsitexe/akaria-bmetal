/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <inttypes.h>

#include <bmetal/drivers/uart.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>

#define REG_RXFIFO    0x00  /* Receiver data FIFO */
#define REG_TXFIFO    0x04  /* Transmit data FIFO */
#define REG_STAT      0x08  /* Status register */
#define REG_CTRL      0x0c  /* Control register */

#define STAT_RXFIFO_VALID    BIT(0)
#define STAT_RXFIFO_FULL     BIT(1)
#define STAT_TXFIFO_EMPTY    BIT(2)
#define STAT_TXFIFO_FULL     BIT(3)
#define STAT_INTR_ENA        BIT(4)
#define STAT_OVERRUN         BIT(5)
#define STAT_FRAME_ERR       BIT(6)
#define STAT_PARITY_ERR      BIT(7)

#define CTRL_RST_TX          BIT(0)
#define CTRL_RST_RX          BIT(1)
#define CTRL_ENA_INTR        BIT(4)

static int uart_lite_add(struct __device *dev)
{
	int r;

	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	return 0;
}

static int uart_lite_remove(struct __device *dev)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}

int uart_lite_char_in(struct __uart_device *uart)
{
	return 0;
}

void uart_lite_char_out(struct __uart_device *uart, int value)
{
	struct __device *dev = __uart_to_dev(uart);

	while ((__device_read32(dev, REG_STAT) & STAT_TXFIFO_FULL) != 0) {
	}

	__device_write32(dev, value, REG_TXFIFO);
}

const static struct __device_driver_ops lite_dev_ops = {
	.add = uart_lite_add,
	.remove = uart_lite_remove,
	.mmap = __device_driver_mmap,
};

const static struct __uart_driver_ops lite_uart_ops = {
	.char_in = uart_lite_char_in,
	.char_out = uart_lite_char_out,
};

static struct __uart_driver lite_drv = {
	.base = {
		.base = {
			.type_vendor = "xilinx",
			.type_device = "uart_lite",
		},

		.ops = &lite_dev_ops,
	},

	.ops = &lite_uart_ops,
};

static int uart_lite_init(void)
{
	__uart_add_driver(&lite_drv);

	return 0;
}

define_init_func(uart_lite_init);
