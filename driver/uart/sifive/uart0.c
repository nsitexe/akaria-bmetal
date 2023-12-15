/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/uart.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>
#include <bmetal/driver/clk.h>
#include <bmetal/driver/intc.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

#define UART_SIFIVE_DEFAULT_BAUD    9600

#define REG_TXDATA     0x00  /* Transmit data */
#define REG_RXDATA     0x04  /* Receive data */
#define REG_TXCTRL     0x08  /* Transmit control */
#define REG_RXCTRL     0x0c  /* Receive control */
#define REG_IE         0x10  /* Interrupt enable */
#define REG_IP         0x14  /* Interrupt pending  */
#define REG_DIV        0x18  /* Baud rate divisor */

#define TXDATA_FULL    BIT(31)

#define RXDATA_EMPTY   BIT(31)

#define TXCTRL_TXEN           BIT(0)
#define TXCTRL_NSTOP          BIT(1)
#define TXCTRL_TXCNT_SHIFT    16
#define TXCTRL_TXCNT_MASK     (0x7 << RXCTRL_RXCNT_SHIFT)

#define RXCTRL_RXEN           BIT(0)
#define RXCTRL_RXCNT_SHIFT    16
#define RXCTRL_RXCNT_MASK     (0x7 << RXCTRL_RXCNT_SHIFT)

#define IE_TXWM               BIT(0)
#define IE_RXWM               BIT(1)

struct uart_sifive_priv {
	struct k_uart_device *uart;
	struct k_clk_device *clk;
	int index_clk;
	uint64_t freq_in;

	struct k_intc_device *intc;
	struct k_event_handler hnd_irq;
	int num_irq;

	struct k_uart_config conf;
};
CHECK_PRIV_SIZE_UART(struct uart_sifive_priv);

static int uart_sifive_char_in(struct k_uart_device *uart)
{
	struct k_device *d = k_uart_to_dev(uart);
	uint32_t v;

	while (((v = k_device_read32(d, REG_RXDATA)) & RXDATA_EMPTY) != 0) {
	}

	return v & 0xffU;
}

static void uart_sifive_char_out(struct k_uart_device *uart, int value)
{
	struct k_device *d = k_uart_to_dev(uart);

	while ((k_device_read32(d, REG_TXDATA) & TXDATA_FULL) != 0) {
	}

	k_device_write32(d, value & 0xffU, REG_TXDATA);
}

static int uart_sifive_enable_intr(struct k_uart_device *uart)
{
	struct k_device *d = k_uart_to_dev(uart);
	uint32_t v;

	//TODO: TX buffering
	v = k_device_read32(d, REG_IE);
	v |= IE_RXWM;
	k_device_write32(d, v, REG_IE);

	return 0;
}

static int uart_sifive_disable_intr(struct k_uart_device *uart)
{
	struct k_device *d = k_uart_to_dev(uart);
	uint32_t v;

	v = k_device_read32(d, REG_IE);
	v &= ~(IE_TXWM | IE_RXWM);
	k_device_write32(d, v, REG_IE);

	return 0;
}

static int uart_sifive_set_baud(struct k_uart_device *uart, uint32_t baud)
{
	struct k_device *dev = k_uart_to_dev(uart);
	struct uart_sifive_priv *priv = dev->priv;
	uint32_t d;

	d = priv->freq_in / baud - 1;
	k_device_write32(dev, d, REG_DIV);

	priv->conf.baud = baud;

	return 0;
}

static int uart_sifive_get_config(struct k_uart_device *uart, struct k_uart_config *conf)
{
	struct k_device *dev = k_uart_to_dev(uart);
	struct uart_sifive_priv *priv = dev->priv;

	if (conf) {
		*conf = priv->conf;
	}

	return 0;
}

static int uart_sifive_set_config(struct k_uart_device *uart, const struct k_uart_config *conf)
{
	int r, res = 0;

	r = uart_sifive_set_baud(uart, conf->baud);
	if (r) {
		res = r;
	}

	return res;
}

static int uart_sifive_intr(int event, struct k_event_handler *hnd)
{
	//struct uart_sifive_priv *priv = hnd->priv;

	//TODO: TX, RX buffering
	return EVENT_HANDLED;
}

static int uart_sifive_add(struct k_device *dev)
{
	struct k_uart_device *uart = k_uart_from_dev(dev);
	struct uart_sifive_priv *priv = dev->priv;
	struct k_uart_config conf;
	uint32_t val;
	int r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->uart = uart;

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

	val = TXCTRL_TXEN |
		(1 << TXCTRL_TXCNT_SHIFT);
	k_device_write32(dev, val, REG_TXCTRL);

	val = RXCTRL_RXEN |
		(0 << RXCTRL_RXCNT_SHIFT);
	k_device_write32(dev, val, REG_RXCTRL);

	/* Interrupt */
	r = k_intc_get_intc_from_config(dev, 0, &priv->intc, &priv->num_irq);
	if (r) {
		k_dev_warn(dev, "intc is not found, use polling.\n");
		priv->intc = NULL;
	}

	if (priv->intc) {
		priv->hnd_irq.func = uart_sifive_intr;
		priv->hnd_irq.priv = priv;

		r = k_intc_add_handler(priv->intc, priv->num_irq, &priv->hnd_irq);
		if (r) {
			return r;
		}

		r = uart_sifive_enable_intr(uart);
		if (r) {
			return r;
		}
	}

	/* Apply board default settings */
	r = k_uart_read_default_config(uart, &conf);
	if (r) {
		return r;
	}
	if (conf.baud == 0) {
		conf.baud = UART_SIFIVE_DEFAULT_BAUD;
	}

	r = uart_sifive_set_config(uart, &conf);
	if (r) {
		return r;
	}

	return 0;
}

static int uart_sifive_remove(struct k_device *dev)
{
	struct k_uart_device *uart = k_uart_from_dev(dev);
	struct uart_sifive_priv *priv = dev->priv;
	int r;

	if (priv->intc) {
		r = uart_sifive_disable_intr(uart);
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

const static struct k_device_driver_ops uart_sifive_dev_ops = {
	.add = uart_sifive_add,
	.remove = uart_sifive_remove,
	.mmap = k_device_driver_mmap,
};

const static struct k_uart_driver_ops uart_sifive_uart_ops = {
	.char_in = uart_sifive_char_in,
	.char_out = uart_sifive_char_out,
	.get_config = uart_sifive_get_config,
	.set_config = uart_sifive_set_config,
};

static struct k_uart_driver uart_sifive_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "uart0",
		},

		.ops = &uart_sifive_dev_ops,
	},

	.ops = &uart_sifive_uart_ops,
};

static int uart_sifive_init(void)
{
	k_uart_add_driver(&uart_sifive_drv);

	return 0;
}
define_init_func(uart_sifive_init);
