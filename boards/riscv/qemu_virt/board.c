/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/bindings/intc/riscv/rv_priv.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/intc.h>
#include <bmetal/drivers/uart.h>

#define CPU_CONF(N)    \
	[N] = { \
		{"hartid", 1, {N}}, \
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
		{"cpu", 1, {UPTR("cpu" #N)}}, \
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
	{"reg", 1, {0x2000000}},
	{"reg-size", 1, {0x1000}},
	{"interrupts", 16, {
		UPTR("rvintc0"), RV_IX_SIX,
		UPTR("rvintc1"), RV_IX_SIX,
		UPTR("rvintc2"), RV_IX_SIX,
		UPTR("rvintc3"), RV_IX_SIX,
		UPTR("rvintc0"), RV_IX_TIX,
		UPTR("rvintc1"), RV_IX_TIX,
		UPTR("rvintc2"), RV_IX_TIX,
		UPTR("rvintc3"), RV_IX_TIX,
		}},
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
	{"reg", 1, {0xc000000}},
	{"reg-size", 1, {0x2000000}},
	{"interrupts", 8, {
		UPTR("rvintc0"), RV_IX_EIX,
		UPTR("rvintc1"), RV_IX_EIX,
		UPTR("rvintc2"), RV_IX_EIX,
		UPTR("rvintc3"), RV_IX_EIX,
		}},
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
	{"reg", 1, {0x10000000}},
	{"reg-size", 1, {0x100}},
	{"reg-width", 1, {1}},
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
	__uart_add_device(&uart0, __bus_get_root(), 1);

	return 0;
}

define_init_func(board_qemu_virt_init);
