/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/uart.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

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

struct uart_lite_priv {
	struct __uart_device *uart;
	struct __intc_device *intc;
	struct __event_handler hnd_irq;
	int num_irq;
};
CHECK_PRIV_SIZE_UART(struct uart_lite_priv);

static int uart_lite_char_in(struct __uart_device *uart)
{
	struct __device *dev = __uart_to_dev(uart);

	while ((__device_read32(dev, REG_STAT) & STAT_RXFIFO_VALID) == 0) {
	}

	return __device_read32(dev, REG_RXFIFO);
}

static void uart_lite_char_out(struct __uart_device *uart, int value)
{
	struct __device *dev = __uart_to_dev(uart);

	while ((__device_read32(dev, REG_STAT) & STAT_TXFIFO_FULL) != 0) {
	}

	__device_write32(dev, value, REG_TXFIFO);
}

static int uart_lite_enable_intr(struct __uart_device *uart)
{
	struct __device *dev = __uart_to_dev(uart);
	uint32_t v;

	v = __device_read32(dev, REG_CTRL);
	v |= CTRL_ENA_INTR;
	__device_write32(dev, v, REG_CTRL);

	return 0;
}

static int uart_lite_disable_intr(struct __uart_device *uart)
{
	struct __device *dev = __uart_to_dev(uart);
	uint32_t v;

	v = __device_read32(dev, REG_CTRL);
	v &= ~CTRL_ENA_INTR;
	__device_write32(dev, v, REG_CTRL);

	return 0;
}

static int uart_lite_intr(int event, struct __event_handler *hnd)
{
	struct uart_lite_priv *priv = hnd->priv;
	struct __device *dev = __uart_to_dev(priv->uart);

	//Clear interrupt
	__device_read32(dev, REG_STAT);

	//TODO: TX, RX buffering
	return EVENT_HANDLED;
}

static int uart_lite_add(struct __device *dev)
{
	struct __uart_device *uart = __uart_from_dev(dev);
	struct uart_lite_priv *priv = dev->priv;
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->uart = uart;

	/* Register */
	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	/* Interrupt */
	r = __intc_get_intc_from_config(dev, 0, &priv->intc, &priv->num_irq);
	if (r) {
		__dev_warn(dev, "intc is not found, use polling.\n");
		priv->intc = NULL;
	}

	if (priv->intc) {
		priv->hnd_irq.func = uart_lite_intr;
		priv->hnd_irq.priv = priv;

		r = __intc_add_handler(priv->intc, priv->num_irq, &priv->hnd_irq);
		if (r) {
			return r;
		}

		r = uart_lite_enable_intr(uart);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int uart_lite_remove(struct __device *dev)
{
	struct __uart_device *uart = __uart_from_dev(dev);
	struct uart_lite_priv *priv = dev->priv;
	int r;

	if (priv->intc) {
		r = uart_lite_disable_intr(uart);
		if (r) {
			return r;
		}

		r = __intc_remove_handler(priv->intc, priv->num_irq, &priv->hnd_irq);
		if (r) {
			return r;
		}

		priv->hnd_irq.func = NULL;
		priv->hnd_irq.priv = NULL;

		priv->num_irq = 0;
		priv->intc = NULL;
	}

	/* TODO: to be implemented */
	return -ENOTSUP;
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
