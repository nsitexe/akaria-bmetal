/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/drivers/timer.h>
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

const static struct __device_config clk_conf[] = {
	PROP("frequency", 25 * MHZ),
	{0},
};

static __clk_priv_t clk_priv;
static struct __clk_device clk = {
	.base = {
		.name = "clk",
		.type_vendor = "none",
		.type_device = "clk_fixed",
		.conf = clk_conf,
		.priv = &clk_priv,
	},
};

const static struct __device_config clint_timer_conf[] = {
	PROP("reg", 0x2004000),
	PROP("reg-size", 0xc000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_TIX),
	PROP("clocks", UPTR("clk"), 0),
	PROP("system", 1),
	{0},
};

static __timer_priv_t clint_timer_priv;
static struct __timer_device clint_timer = {
	.base = {
		.name = "clint_timer",
		.type_vendor = "sifive",
		.type_device = "clint0_timer",
		.conf = clint_timer_conf,
		.priv = &clint_timer_priv,
	}
};

const static struct __device_config uart0_conf[] = {
	PROP("reg", 0xe4006000),
	PROP("reg-size", 0x1000),
	PROP("baud", 9600),
	{0},
};

static __uart_priv_t uart0_priv;
static struct __uart_device uart0 = {
	.base = {
		.name = "uart0",
		.type_vendor = "xilinx",
		.type_device = "uart_lite",
		.conf = uart0_conf,
		.priv = &uart0_priv,
	},
};

static int board_ns31_arty_init(void)
{
	__cpu_add_device(&cpu0, __bus_get_root());
	__clk_add_device(&clk, __bus_get_root());
	__timer_add_device(&clint_timer, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_ns31_arty_init);
