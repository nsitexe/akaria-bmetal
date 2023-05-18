/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/drivers/timer.h>
#include <bmetal/drivers/uart.h>

#define CPU_CONF(N)    \
	{ \
		PROP("hartid", N), \
		{0}, \
	}

#define CPU_DEVICE(N, INDEX)    \
	{ \
		.base = { \
			.name = "cpu" #N, \
			.type_vendor = "none", \
			.type_device = "cpu_riscv", \
			.conf = cpu_conf[INDEX], \
			.priv = &cpu_priv[INDEX], \
		}, \
	}

const static struct __device_config cpu_conf[][2] = {
	CPU_CONF(0),
};

static __cpu_priv_t cpu_priv[1];
static struct __cpu_device cpu[] = {
	CPU_DEVICE(0, 0),
};
CHECK_ELEM_SIZE(cpu, cpu_conf);
CHECK_ELEM_SIZE(cpu, cpu_priv);

#define INTC_CONF(N)    \
	{ \
		PROP("cpu", UPTR("cpu" #N)), \
		{0}, \
	}

#define INTC_DEVICE(N, INDEX)    \
	{ \
		.base = { \
			.name = "rvintc" #N, \
			.type_vendor = "riscv", \
			.type_device = "priv1_10", \
			.conf = rvintc_conf[INDEX], \
			.priv = &rvintc_priv[INDEX], \
		}, \
	}

const static struct __device_config rvintc_conf[][2] = {
	INTC_CONF(0),
};

static __intc_priv_t rvintc_priv[1];
static struct __intc_device rvintc[] = {
	INTC_DEVICE(0, 0),
};
CHECK_ELEM_SIZE(rvintc, rvintc_conf);
CHECK_ELEM_SIZE(rvintc, rvintc_priv);

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

const static struct __device_config plic_conf[] = {
	PROP("reg", 0xc000000),
	PROP("reg-size", 0x2000000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_EIX),
	{0},
};

static __intc_priv_t plic_priv;
static struct __intc_device plic = {
	.base = {
		.name = "plic",
		.type_vendor = "sifive",
		.type_device = "plic0",
		.conf = plic_conf,
		.priv = &plic_priv,
	}
};

const static struct __device_config uart0_conf[] = {
	PROP("reg", 0xe4006000),
	PROP("reg-size", 0x1000),
	PROP("interrupts", UPTR("plic"), 9),
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
	__cpu_add_device(&cpu[0], __bus_get_root());
	__intc_add_device(&rvintc[0], __bus_get_root());
	__intc_add_device(&clint, __bus_get_root());
	__intc_add_device(&plic, __bus_get_root());
	__clk_add_device(&clk, __bus_get_root());
	__timer_add_device(&clint_timer, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_ns31_arty_init);
