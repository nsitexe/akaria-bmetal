/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/drivers/cpu.h>
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
	for (int i = 0; i < 4; i++) {
		__cpu_add_device(&cpu[i], __bus_get_root());
	}
	__uart_add_device(&uart0, __bus_get_root(), 1);

	return 0;
}

define_init_func(board_qemu_virt_init);
