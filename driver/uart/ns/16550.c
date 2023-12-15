/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/uart.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>
#include <bmetal/driver/clk.h>
#include <bmetal/driver/intc.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

#define UART_NS16550_DEFAULT_BAUD    9600

#define REG_RBR     0x00  /* Receive buffer      */
#define REG_THR     0x00  /* Transmit holding    */
#define REG_IER     0x01  /* Interrupt enable    */
#define REG_IIR     0x02  /* Interrupt ID        */
#define REG_FCR     0x02  /* FIFO control        */
#define REG_LCR     0x03  /* Line control        */
#define REG_MCR     0x04  /* Modem control       */
#define REG_LSR     0x05  /* Line status         */
#define REG_MSR     0x06  /* Modem status        */
#define REG_SCR     0x07  /* Scratch pad         */

/* DLAB1 (LCR[7] = 1) */
#define REG_DLL     0x00  /* Divisor latch (LSB) */
#define REG_DLM     0x01  /* Divisor latch (MSB) */
#define REG_AFR     0x02  /* Alternate function  */

#define IER_ERBFI              BIT(0) /* Receive data available */
#define IER_ETBEI              BIT(1) /* Transmitter empty */
#define IER_ELSI               BIT(2) /* Line status */
#define IER_EDSSI              BIT(3) /* Modem status */

#define LSR_RX_READY           BIT(0)
#define LSR_RX_OVERRUN_ERR     BIT(1)
#define LSR_RX_PARITY_ERR      BIT(2)
#define LSR_RX_FRAME_ERR       BIT(3)
#define LSR_RX_BREAK           BIT(4)
#define LSR_THR_EMPTY          BIT(5)
#define LSR_THR_TSR_EMPTY      BIT(6)
#define LSR_RX_FIFO_GBL_ERR    BIT(7)

#define REG_WIDTH_DEFAULT    4

struct uart_ns16550_priv {
	struct k_uart_device *uart;
	struct k_clk_device *clk;
	int index_clk;
	uint64_t freq_in;

	uint32_t addr_shift;

	struct k_intc_device *intc;
	struct k_event_handler hnd_irq;
	int num_irq;

	struct k_uart_config conf;
};
CHECK_PRIV_SIZE_UART(struct uart_ns16550_priv);

static uint8_t uart_read(struct k_uart_device *uart, uintptr_t off)
{
	struct k_device *dev = k_uart_to_dev(uart);
	struct uart_ns16550_priv *priv = dev->priv;
	uintptr_t shifted = off << priv->addr_shift;

	switch (priv->addr_shift) {
	case 0:
		return k_device_read8(k_uart_to_dev(uart), shifted);
	case 1:
		return k_device_read16(k_uart_to_dev(uart), shifted);
	case 2:
		return k_device_read32(k_uart_to_dev(uart), shifted);
	default:
		/* BUG */
		return -1;
	}
}

static void uart_write(struct k_uart_device *uart, uint8_t dat, uintptr_t off)
{
	struct k_device *dev = k_uart_to_dev(uart);
	struct uart_ns16550_priv *priv = dev->priv;
	uintptr_t shifted = off << priv->addr_shift;

	switch (priv->addr_shift) {
	case 0:
		k_device_write8(dev, dat, shifted);
		break;
	case 1:
		k_device_write16(dev, dat, shifted);
		break;
	case 2:
		k_device_write32(dev, dat, shifted);
		break;
	default:
		/* BUG */
		return;
	}
}

static int uart_ns16550_set_baud(struct k_uart_device *uart, uint32_t baud)
{
	struct k_device *dev = k_uart_to_dev(uart);
	struct uart_ns16550_priv *priv = dev->priv;
	uint32_t d;
	uint8_t v;

	d = priv->freq_in / baud / 16;
	if (d == 0) {
		/* Out of range, input clock rate is too low */
		k_dev_err(dev, "Divisor latch is zero (baud:%d)\n", (int)baud);
		return -EINVAL;
	}
	if (d > 0xffff) {
		/* Out of range, input clock rate is too high */
		k_dev_err(dev, "Not support baud:%d\n", (int)baud);
		return -EINVAL;
	}

	/* Enable divisor latch */
	v = uart_read(uart, REG_LCR);
	uart_write(uart, v | 0x80, REG_LCR);

	uart_write(uart, d, REG_DLL);
	uart_write(uart, d >> 8, REG_DLM);

	/* Disable divisor latch */
	v = uart_read(uart, REG_LCR);
	uart_write(uart, v & ~0x80, REG_LCR);

	return 0;
}

static int uart_ns16550_char_in(struct k_uart_device *uart)
{
	while ((uart_read(uart, REG_LSR) & LSR_RX_READY) == 0) {
	}

	return uart_read(uart, REG_RBR);
}

static void uart_ns16550_char_out(struct k_uart_device *uart, int value)
{
	while ((uart_read(uart, REG_LSR) & LSR_THR_EMPTY) == 0) {
	}

	uart_write(uart, value, REG_THR);
}

static int uart_ns16550_enable_intr(struct k_uart_device *uart)
{
	uint8_t v;

	v = uart_read(uart, REG_IER);
	v |= IER_ERBFI | IER_ETBEI;
	uart_write(uart, v, REG_IER);

	return 0;
}

static int uart_ns16550_disable_intr(struct k_uart_device *uart)
{
	uint8_t v;

	v = uart_read(uart, REG_IER);
	v &= ~(IER_ERBFI | IER_ETBEI);
	uart_write(uart, v, REG_IER);

	return 0;
}

static int uart_ns16550_get_config(struct k_uart_device *uart, struct k_uart_config *conf)
{
	struct k_device *dev = k_uart_to_dev(uart);
	struct uart_ns16550_priv *priv = dev->priv;

	if (conf) {
		*conf = priv->conf;
	}

	return 0;
}

static int uart_ns16550_set_config(struct k_uart_device *uart, const struct k_uart_config *conf)
{
	int r, res = 0;

	r = uart_ns16550_set_baud(uart, conf->baud);
	if (r) {
		res = r;
	}

	return res;
}

static int uart_ns16550_intr(int event, struct k_event_handler *hnd)
{
	struct uart_ns16550_priv *priv = hnd->priv;
	struct k_uart_device *uart = priv->uart;
	int r;

	//TODO: currently just ignore interrupts
	r = uart_ns16550_disable_intr(uart);
	if (r) {
		return EVENT_NOT_HANDLED;
	}

	//TODO: TX, RX buffering
	return EVENT_HANDLED;
}

static int uart_ns16550_add(struct k_device *dev)
{
	struct k_uart_device *uart = k_uart_from_dev(dev);
	struct uart_ns16550_priv *priv = dev->priv;
	struct k_uart_config conf;
	uint32_t w;
	int r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->uart = uart;

	/* Some 16550 variant have different register width */
	r = k_device_read_conf_u32(dev, "reg-width", &w, 0);
	if (r) {
		w = REG_WIDTH_DEFAULT;

		k_dev_err(dev, "config 'reg-width' is not found."
			"Use default size %"PRId32".\n", w);
	}

	switch (w) {
	case 1:
		priv->addr_shift = 0;
		break;
	case 2:
		priv->addr_shift = 1;
		break;
	case 4:
		priv->addr_shift = 2;
		break;
	default:
		k_dev_err(dev, "config 'reg-width' %"PRId32" is not supported.\n", w);
		return -EINVAL;
	}

	/* Clock */
	r = k_clk_get_clk_from_config(dev, 0, &priv->clk, &priv->index_clk);
	if (r) {
		return r;
	}

	r = k_clk_get_frequency(priv->clk, priv->index_clk, &priv->freq_in);
	if (r) {
		k_dev_err(dev, "clock freq is unknown.\n");
		return r;
	}

	r = k_clk_enable(priv->clk, priv->index_clk);
	if (r) {
		return r;
	}

	/* Register */
	r = k_io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	/* Interrupt */
	r = k_intc_get_intc_from_config(dev, 0, &priv->intc, &priv->num_irq);
	if (r) {
		k_dev_warn(dev, "intc is not found, use polling.\n");
		priv->intc = NULL;
	}

	if (priv->intc) {
		priv->hnd_irq.func = uart_ns16550_intr;
		priv->hnd_irq.priv = priv;

		r = k_intc_add_handler(priv->intc, priv->num_irq, &priv->hnd_irq);
		if (r) {
			return r;
		}

		r = uart_ns16550_enable_intr(uart);
		if (r) {
			return r;
		}
	}

	/* Apply board default UART settings */
	r = k_uart_read_default_config(uart, &conf);
	if (r) {
		return r;
	}
	if (conf.baud == 0) {
		conf.baud = UART_NS16550_DEFAULT_BAUD;
	}

	r = uart_ns16550_set_config(uart, &conf);
	if (r) {
		return r;
	}

	return 0;
}

static int uart_ns16550_remove(struct k_device *dev)
{
	struct k_uart_device *uart = k_uart_from_dev(dev);
	struct uart_ns16550_priv *priv = dev->priv;
	int r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	if (priv->intc) {
		r = uart_ns16550_disable_intr(uart);
		if (r) {
			return r;
		}

		r = k_intc_remove_handler(priv->intc, priv->num_irq, &priv->hnd_irq);
		if (r) {
			return r;
		}

		priv->hnd_irq.func = NULL;
		priv->hnd_irq.priv = NULL;

		priv->num_irq = 0;
		priv->intc = NULL;
	}

	r = k_clk_disable(priv->clk, priv->index_clk);
	if (r) {
		return r;
	}

	/* TODO: to be implemented */
	return -ENOTSUP;
}

const static struct k_device_driver_ops ns16550_dev_ops = {
	.add = uart_ns16550_add,
	.remove = uart_ns16550_remove,
	.mmap = k_device_driver_mmap,
};

const static struct k_uart_driver_ops ns16550_uart_ops = {
	.char_in = uart_ns16550_char_in,
	.char_out = uart_ns16550_char_out,
	.get_config = uart_ns16550_get_config,
	.set_config = uart_ns16550_set_config,
};

static struct k_uart_driver ns16550_drv = {
	.base = {
		.base = {
			.type_vendor = "ns",
			.type_device = "ns16550",
		},

		.ops = &ns16550_dev_ops,
	},

	.ops = &ns16550_uart_ops,
};

static int uart_ns16550_init(void)
{
	k_uart_add_driver(&ns16550_drv);

	return 0;
}
define_init_func(uart_ns16550_init);
