/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/drivers/cpu.h>
#include <bmetal/drivers/uart.h>

const static struct __device_config cpu0_conf[] = {
	{"hartid", 0},
	{0, 0},
};

const static struct __device_config cpu1_conf[] = {
	{"hartid", 1},
	{0, 0},
};

const static struct __device_config cpu2_conf[] = {
	{"hartid", 2},
	{0, 0},
};

const static struct __device_config cpu3_conf[] = {
	{"hartid", 3},
	{0, 0},
};

static struct __cpu_device cpu0 = {
	.base = {
		.name = "cpu0",
		.type_vendor = "generic",
		.type_device = "cpu_riscv",
		.conf = cpu0_conf,
	},
};

static struct __cpu_device cpu1 = {
	.base = {
		.name = "cpu1",
		.type_vendor = "generic",
		.type_device = "cpu_riscv",
		.conf = cpu1_conf,
	},
};

static struct __cpu_device cpu2 = {
	.base = {
		.name = "cpu2",
		.type_vendor = "generic",
		.type_device = "cpu_riscv",
		.conf = cpu2_conf,
	},
};

static struct __cpu_device cpu3 = {
	.base = {
		.name = "cpu3",
		.type_vendor = "generic",
		.type_device = "cpu_riscv",
		.conf = cpu3_conf,
	},
};

const static struct __device_config uart0_conf[] = {
	{"reg", 0x10000000},
	{"reg-size", 0x100},
	{"reg-width", 1},
	{0, 0},
};

static struct __uart_device uart0 = {
	.base = {
		.name = "uart0",
		.type_vendor = "generic",
		.type_device = "uart_ns16550",
		.conf = uart0_conf,
	},
};

static int board_qemu_virt_init(void)
{
	__cpu_add_device(&cpu0, __bus_get_root());
	__cpu_add_device(&cpu1, __bus_get_root());
	__cpu_add_device(&cpu2, __bus_get_root());
	__cpu_add_device(&cpu3, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);

	return 0;
}

define_init_func(board_qemu_virt_init);
