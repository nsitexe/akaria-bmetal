/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/uart.h>

const static struct __device_config cpu0_conf[] = {
	PROP("hartid", 0),
	{0},
};

static __cpu_priv_t cpu0_priv;
static struct __cpu_device cpu0 = {
	.base = {
		.name = "cpu0",
		.type_vendor = "none",
		.type_device = "cpu_riscv",
		.conf = cpu0_conf,
		.priv = &cpu0_priv,
	},
};

const static struct __device_config hfclk_conf[] = {
	PROP("frequency", 16000000),
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

const static struct __device_config uart0_conf[] = {
	PROP("reg", 0x10013000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("hfclk"), 0),
	{0},
};

const static struct __device_config uart1_conf[] = {
	PROP("reg", 0x10023000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("hfclk"), 0),
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

static int board_hifive1_init(void)
{
	__cpu_add_device(&cpu0, __bus_get_root());
	__clk_add_device(&hfclk, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);
	__uart_add_device(&uart1, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_hifive1_init);
