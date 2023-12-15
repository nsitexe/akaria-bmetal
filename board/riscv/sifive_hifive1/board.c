/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/driver/clk.h>
#include <bmetal/driver/cpu.h>
#include <bmetal/driver/intc.h>
#include <bmetal/driver/timer.h>
#include <bmetal/driver/uart.h>

const static struct k_device_config cpu0_conf[] = {
	PROP("hartid", 0),
	{0},
};

static k_cpu_priv_t cpu0_priv;
static struct k_cpu_device cpu0 = {
	.base = {
		.name = "cpu0",
		.type_vendor = "none",
		.type_device = "cpu_riscv",
		.conf = cpu0_conf,
		.priv = &cpu0_priv,
	},
};

const static struct k_device_config rvintc0_conf[] = {
	PROP("cpu", UPTR("cpu0")),
	{0},
};

static k_intc_priv_t rvintc0_priv;
static struct k_intc_device rvintc0 = {
	.base = {
		.name = "rvintc0",
		.type_vendor = "riscv",
		.type_device = "priv1_10",
		.conf = rvintc0_conf,
		.priv = &rvintc0_priv,
	},
};

const static struct k_device_config clint_conf[] = {
	PROP("reg", 0x2000000),
	PROP("reg-size", 0x1000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_SIX),
	PROP("ipi", 1),
	{0},
};

static k_intc_priv_t clint_priv;
static struct k_intc_device clint = {
	.base = {
		.name = "clint",
		.type_vendor = "sifive",
		.type_device = "clint0",
		.conf = clint_conf,
		.priv = &clint_priv,
	}
};

const static struct k_device_config lfclk_conf[] = {
	PROP("frequency", 32 * KHZ),
	{0},
};

static k_clk_priv_t lfclk_priv;
static struct k_clk_device lfclk = {
	.base = {
		.name = "lfclk",
		.type_vendor = "none",
		.type_device = "clk_fixed",
		.conf = lfclk_conf,
		.priv = &lfclk_priv,
	},
};

const static struct k_device_config clint_timer_conf[] = {
	PROP("reg", 0x2004000),
	PROP("reg-size", 0xc000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_TIX),
	PROP("clocks", UPTR("lfclk"), 0),
	PROP("system", 1),
	{0},
};

static k_timer_priv_t clint_timer_priv;
static struct k_timer_device clint_timer = {
	.base = {
		.name = "clint_timer",
		.type_vendor = "sifive",
		.type_device = "clint0_timer",
		.conf = clint_timer_conf,
		.priv = &clint_timer_priv,
	}
};

const static struct k_device_config hfclk_conf[] = {
	PROP("frequency", 16 * MHZ),
	{0},
};

static k_clk_priv_t hfclk_priv;
static struct k_clk_device hfclk = {
	.base = {
		.name = "hfclk",
		.type_vendor = "none",
		.type_device = "clk_fixed",
		.conf = hfclk_conf,
		.priv = &hfclk_priv,
	},
};

const static struct k_device_config uart0_conf[] = {
	PROP("reg", 0x10013000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("hfclk"), 0),
	PROP("baud", 115200),
	{0},
};

const static struct k_device_config uart1_conf[] = {
	PROP("reg", 0x10023000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("hfclk"), 0),
	PROP("baud", 115200),
	{0},
};

static k_uart_priv_t uart0_priv;
static struct k_uart_device uart0 = {
	.base = {
		.name = "uart0",
		.type_vendor = "sifive",
		.type_device = "uart0",
		.conf = uart0_conf,
		.priv = &uart0_priv,
	},
};

static k_uart_priv_t uart1_priv;
static struct k_uart_device uart1 = {
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
	k_cpu_add_device(&cpu0, k_bus_get_root());
	k_intc_add_device(&rvintc0, k_bus_get_root());
	k_intc_add_device(&clint, k_bus_get_root());
	k_clk_add_device(&lfclk, k_bus_get_root());
	k_clk_add_device(&hfclk, k_bus_get_root());
	k_timer_add_device(&clint_timer, k_bus_get_root());
	k_uart_add_device(&uart0, k_bus_get_root(), 1);
	k_uart_add_device(&uart1, k_bus_get_root(), 1);

	k_uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_hifive1_init);
