/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/bindings/intc/riscv/rv_priv.h>
#include <bmetal/drivers/clk.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/drivers/timer.h>
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
};

static __cpu_priv_t cpu_priv[4];
static struct __cpu_device cpu[] = {
	CPU_DEVICE(0),
	CPU_DEVICE(1),
	CPU_DEVICE(2),
	CPU_DEVICE(3),
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
};

static __intc_priv_t rvintc_priv[4];
static struct __intc_device rvintc[] = {
	INTC_DEVICE(0),
	INTC_DEVICE(1),
	INTC_DEVICE(2),
	INTC_DEVICE(3),
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

const static struct __device_config plic_conf[] = {
	PROP("reg", 0xc000000),
	PROP("reg-size", 0x2000000),
	PROP("interrupts",
		UPTR("rvintc0"), RV_IX_EIX,
		UPTR("rvintc1"), RV_IX_EIX,
		UPTR("rvintc2"), RV_IX_EIX,
		UPTR("rvintc3"), RV_IX_EIX),
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

const static struct __device_config rtcclk_conf[] = {
	PROP("frequency", 10 * MHZ),
	{0},
};

static __clk_priv_t rtcclk_priv;
static struct __clk_device rtcclk = {
	.base = {
		.name = "rtcclk",
		.type_vendor = "none",
		.type_device = "clk_fixed",
		.conf = rtcclk_conf,
		.priv = &rtcclk_priv,
	},
};

const static struct __device_config clint_timer_conf[] = {
	PROP("reg", 0x2004000),
	PROP("reg-size", 0xc000),
	PROP("clocks", UPTR("rtcclk"), 0),
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
	PROP("reg", 0x10000000),
	PROP("reg-size", 0x100),
	PROP("reg-width", 1),
	{0},
};

static struct __uart_device uart0 = {
	.base = {
		.name = "uart0",
		.type_vendor = "ns",
		.type_device = "ns16550",
		.conf = uart0_conf,
	},
};

static int board_qemu_virt_init(void)
{
	for (int i = 0; i < ARRAY_OF(cpu); i++) {
		__cpu_add_device(&cpu[i], __bus_get_root());
		__intc_add_device(&rvintc[i], __bus_get_root());
	}
	__intc_add_device(&plic, __bus_get_root());
	__intc_add_device(&clint, __bus_get_root());
	__clk_add_device(&rtcclk, __bus_get_root());
	__timer_add_device(&clint_timer, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);

	return 0;
}

define_init_func(board_qemu_virt_init);
