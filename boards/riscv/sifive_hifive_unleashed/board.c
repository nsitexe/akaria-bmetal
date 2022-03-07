/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/bindings/clk/sifive/prci.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/uart.h>

const static struct __device_config cpu0_conf[] = {
	{"hartid", 1, {0}},
	{0},
};

static struct __cpu_device cpu0 = {
	.base = {
		.name = "cpu0",
		.type_vendor = "none",
		.type_device = "cpu_riscv",
		.conf = cpu0_conf,
	},
};

const static struct __device_config hfclk_conf[] = {
	{"frequency", 1, {33333333}},
	{0},
};

static __clk_priv_t hfclk_priv;
static struct __clk_device hfclk = {
	.base = {
		.name = "hfclk",
		.type_vendor = "none",
		.type_device = "clk_fixed",
		.conf = hfclk_conf,
		.priv = &hfclk_priv,
	},
};

const static struct __device_config rtclk_conf[] = {
	{"frequency", 1, {1 * MHZ}},
	{0},
};

static __clk_priv_t rtclk_priv;
static struct __clk_device rtclk = {
	.base = {
		.name = "rtclk",
		.type_vendor = "none",
		.type_device = "clk_fixed",
		.conf = rtclk_conf,
		.priv = &rtclk_priv,
	},
};

/* ddrctrlclk, tlclk, coreclk, gemgxlclk */
const static struct __device_config prci_conf[] = {
	{"reg", 1, {0x10000000}},
	{"reg-size", 1, {0x1000}},
	{"clocks", 2, {UPTR("hfclk"), 0}},
	/* Cannot set tlclk (fixed as coreclk / 2) */
	{"frequency", 4, {600 * MHZ, 0, 1 * GHZ, 125 * MHZ}},
	{0},
};

static __clk_priv_t prci_priv;
static struct __clk_device prci = {
	.base = {
		.name = "prci",
		.type_vendor = "sifive",
		.type_device = "prci0",
		.conf = prci_conf,
		.priv = &prci_priv,
	},
};

const static struct __device_config uart0_conf[] = {
	{"reg", 1, {0x10010000}},
	{"reg-size", 1, {0x1000}},
	{"clocks", 2, {UPTR("prci"), PRCI_INDEX_TLCLK}},
	{0},
};

const static struct __device_config uart1_conf[] = {
	{"reg", 1, {0x10011000}},
	{"reg-size", 1, {0x1000}},
	{"clocks", 2, {UPTR("prci"), PRCI_INDEX_TLCLK}},
	{0},
};

static __uart_priv_t uart0_priv;
static struct __uart_device uart0 = {
	.base = {
		.name = "uart0",
		.type_vendor = "sifive",
		.type_device = "uart0",
		.conf = uart0_conf,
		.priv = &uart0_priv,
	},
};

static __uart_priv_t uart1_priv;
static struct __uart_device uart1 = {
	.base = {
		.name = "uart1",
		.type_vendor = "sifive",
		.type_device = "uart0",
		.conf = uart1_conf,
		.priv = &uart1_priv,
	},
};

static int board_hifive_unleashed_init(void)
{
	__cpu_add_device(&cpu0, __bus_get_root());
	__clk_add_device(&hfclk, __bus_get_root());
	__clk_add_device(&rtclk, __bus_get_root());
	__clk_add_device(&prci, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);
	__uart_add_device(&uart1, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_hifive_unleashed_init);
