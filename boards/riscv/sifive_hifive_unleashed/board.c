/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/bindings/clk/sifive/prci.h>
#include <bmetal/bindings/intc/riscv/rv_priv.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/drivers/uart.h>

#define CPU_CONF(N)    \
	[N] = { \
		PROP("hartid", N), \
		{0}, \
	}

#define CPU_DEVICE(N)    \
	[N] = { \
		.base = { \
			.name = "cpu" #N, \
			.type_vendor = "none", \
			.type_device = "cpu_riscv", \
			.conf = cpu_conf[N], \
			.priv = &cpu_priv[N], \
		}, \
	}

const static struct __device_config cpu_conf[][2] = {
	CPU_CONF(0),
	CPU_CONF(1),
	CPU_CONF(2),
	CPU_CONF(3),
	CPU_CONF(4),
};

static __cpu_priv_t cpu_priv[5];
static struct __cpu_device cpu[] = {
	CPU_DEVICE(0),
	CPU_DEVICE(1),
	CPU_DEVICE(2),
	CPU_DEVICE(3),
	CPU_DEVICE(4),
};

#define INTC_CONF(N)    \
	[N] = { \
		PROP("cpu", UPTR("cpu" #N)), \
		{0}, \
	}

#define INTC_DEVICE(N)    \
	[N] = { \
		.base = { \
			.name = "rvintc" #N, \
			.type_vendor = "riscv", \
			.type_device = "priv1_10", \
			.conf = rvintc_conf[N], \
			.priv = &rvintc_priv[N], \
		}, \
	}

const static struct __device_config rvintc_conf[][2] = {
	INTC_CONF(0),
	INTC_CONF(1),
	INTC_CONF(2),
	INTC_CONF(3),
	INTC_CONF(4),
};

static __intc_priv_t rvintc_priv[5];
static struct __intc_device rvintc[] = {
	INTC_DEVICE(0),
	INTC_DEVICE(1),
	INTC_DEVICE(2),
	INTC_DEVICE(3),
	INTC_DEVICE(4),
};

const static struct __device_config clint_conf[] = {
	PROP("reg", 0x2000000),
	PROP("reg-size", 0x1000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_SIX,
		UPTR("rvintc1"), RV_IX_SIX,
		UPTR("rvintc2"), RV_IX_SIX,
		UPTR("rvintc3"), RV_IX_SIX,
		UPTR("rvintc0"), RV_IX_TIX,
		UPTR("rvintc1"), RV_IX_TIX,
		UPTR("rvintc2"), RV_IX_TIX,
		UPTR("rvintc3"), RV_IX_TIX),
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

const static struct __device_config hfclk_conf[] = {
	PROP("frequency", 33333333),
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
	PROP("frequency", 1 * MHZ),
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
	PROP("reg", 0x10000000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("hfclk"), 0),
	/* Cannot set tlclk (fixed as coreclk / 2) */
	PROP("frequency", 600 * MHZ, 0, 1 * GHZ, 125 * MHZ),
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
	PROP("reg", 0x10010000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("prci"), PRCI_INDEX_TLCLK),
	{0},
};

const static struct __device_config uart1_conf[] = {
	PROP("reg", 0x10011000),
	PROP("reg-size", 0x1000),
	PROP("clocks", UPTR("prci"), PRCI_INDEX_TLCLK),
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
	for (int i = 0; i < ARRAY_OF(cpu); i++) {
		__cpu_add_device(&cpu[i], __bus_get_root());
		__intc_add_device(&rvintc[i], __bus_get_root());
	}
	__intc_add_device(&clint, __bus_get_root());
	__clk_add_device(&hfclk, __bus_get_root());
	__clk_add_device(&rtclk, __bus_get_root());
	__clk_add_device(&prci, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);
	__uart_add_device(&uart1, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_hifive_unleashed_init);
