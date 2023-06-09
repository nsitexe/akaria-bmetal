/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/drivers/uart.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>
#include <bmetal/drivers/clk.h>
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

struct uart_sifive_priv {
	struct __clk_device *clk;
	int index_clk;
	uint64_t freq_in;
	struct __uart_config conf;
};
CHECK_PRIV_SIZE_UART(struct uart_sifive_priv);

int uart_sifive_char_in(struct __uart_device *uart)
{
	struct __device *d = __uart_to_dev(uart);
	uint32_t v;

	while (((v = __device_read32(d, REG_RXDATA)) & RXDATA_EMPTY) != 0) {
	}

	return v & 0xffU;
}

void uart_sifive_char_out(struct __uart_device *uart, int value)
{
	struct __device *d = __uart_to_dev(uart);

	while ((__device_read32(d, REG_TXDATA) & TXDATA_FULL) != 0) {
	}

	__device_write32(d, value & 0xffU, REG_TXDATA);
}

static int uart_sifive_set_baud(struct __uart_device *uart, uint32_t baud)
{
	struct __device *dev = __uart_to_dev(uart);
	struct uart_sifive_priv *priv = dev->priv;
	uint32_t d;

	d = priv->freq_in / baud - 1;
	__device_write32(dev, d, REG_DIV);

	priv->conf.baud = baud;

	return 0;
}

static int uart_sifive_get_config(struct __uart_device *uart, struct __uart_config *conf)
{
	struct __device *dev = __uart_to_dev(uart);
	struct uart_sifive_priv *priv = dev->priv;

	if (conf) {
		*conf = priv->conf;
	}

	return 0;
}

static int uart_sifive_set_config(struct __uart_device *uart, const struct __uart_config *conf)
{
	int r, res = 0;

	r = uart_sifive_set_baud(uart, conf->baud);
	if (r) {
		res = r;
	}

	return res;
}

static int uart_sifive_add(struct __device *dev)
{
	struct __uart_device *uart = __uart_from_dev(dev);
	struct uart_sifive_priv *priv = dev->priv;
	struct __uart_config conf;
	uint32_t val;
	int r;

	if (priv == NULL) {
		__dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	r = __clk_get_clk_from_config(dev, 0, &priv->clk, &priv->index_clk);
	if (r) {
		return r;
	}

	r = __clk_get_frequency(priv->clk, priv->index_clk, &priv->freq_in);
	if (r) {
		__dev_err(dev, "clock freq is unknown.\n");
		return r;
	}

	r = __clk_enable(priv->clk, priv->index_clk);
	if (r) {
		return r;
	}

	r = __io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	val = TXCTRL_TXEN |
		(1 << TXCTRL_TXCNT_SHIFT);
	__device_write32(dev, val, REG_TXCTRL);

	val = RXCTRL_RXEN |
		(0 << RXCTRL_RXCNT_SHIFT);
	__device_write32(dev, val, REG_RXCTRL);

	/* Apply board default settings */
	r = __uart_read_default_config(uart, &conf);
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

static int uart_sifive_remove(struct __device *dev)
{
	/* TODO: to be implemented */
	return -ENOTSUP;
}

const static struct __device_driver_ops uart_sifive_dev_ops = {
	.add = uart_sifive_add,
	.remove = uart_sifive_remove,
	.mmap = __device_driver_mmap,
};

const static struct __uart_driver_ops uart_sifive_uart_ops = {
	.char_in = uart_sifive_char_in,
	.char_out = uart_sifive_char_out,
	.get_config = uart_sifive_get_config,
	.set_config = uart_sifive_set_config,
};

static struct __uart_driver uart_sifive_drv = {
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
	__uart_add_driver(&uart_sifive_drv);

	return 0;
}
define_init_func(uart_sifive_init);
