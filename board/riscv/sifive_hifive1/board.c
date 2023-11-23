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

const static struct __device_config rvintc0_conf[] = {
	PROP("cpu", UPTR("cpu0")),
	{0},
};

static __intc_priv_t rvintc0_priv;
static struct __intc_device rvintc0 = {
	.base = {
		.name = "rvintc0",
		.type_vendor = "riscv",
		.type_device = "priv1_10",
		.conf = rvintc0_conf,
		.priv = &rvintc0_priv,
	},
};

const static struct __device_config clint_conf[] = {
	PROP("reg", 0x2000000),
	PROP("reg-size", 0x1000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_SIX),
	PROP("ipi", 1),
	{0},
};

static __intc_priv_t clint_priv;
static struct __intc_device clint = {
	.base = {
		.name = "clint",
		.type_vendor = "sifive",
		.type_device = "clint0",
		.conf = clint_conf,
		.priv = &clint_priv,
	}
};

const static struct __device_config lfclk_conf[] = {
	PROP("frequency", 32 * KHZ),
	{0},
};

static __clk_priv_t lfclk_priv;
static struct __clk_device lfclk = {
	.base = {
		.name = "lfclk",
		.type_vendor = "none",
		.type_device = "clk_fixed",
		.conf = lfclk_conf,
		.priv = &lfclk_priv,
	},
};

const static struct __device_config clint_timer_conf[] = {
	PROP("reg", 0x2004000),
	PROP("reg-size", 0xc000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_TIX),
	PROP("clocks", UPTR("lfclk"), 0),
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

const static struct __device_config hfclk_conf[] = {
	PROP("frequency", 16 * MHZ),
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
	PROP("baud", 115200),
	{0},
};

const static struct __device_config uart1_conf[] = {
	PROP("reg", 0x10023000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("hfclk"), 0),
	PROP("baud", 115200),
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
	k_cpu_add_device(&cpu0, __bus_get_root());
	__intc_add_device(&rvintc0, __bus_get_root());
	__intc_add_device(&clint, __bus_get_root());
	__clk_add_device(&lfclk, __bus_get_root());
	__clk_add_device(&hfclk, __bus_get_root());
	__timer_add_device(&clint_timer, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);
	__uart_add_device(&uart1, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_hifive1_init);
