/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
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
	{"frequency", 1, {1000000}},
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

const static struct __device_config uart0_conf[] = {
	{"reg", 1, {0x10010000}},
	{"reg-size", 1, {0x1000}},
	{0},
};

const static struct __device_config uart1_conf[] = {
	{"reg", 1, {0x10011000}},
	{"reg-size", 1, {0x1000}},
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

static int board_qemu_virt_init(void)
{
	__cpu_add_device(&cpu0, __bus_get_root());
	__clk_add_device(&hfclk, __bus_get_root());
	__clk_add_device(&rtclk, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);
	__uart_add_device(&uart1, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_qemu_virt_init);
