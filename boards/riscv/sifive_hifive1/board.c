/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
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

const static struct __device_config uart0_conf[] = {
	{"reg", 1, {0x10013000}},
	{"reg-size", 1, {0x1000}},
	{0},
};

const static struct __device_config uart1_conf[] = {
	{"reg", 1, {0x10023000}},
	{"reg-size", 1, {0x1000}},
	{0},
};

static struct __uart_device uart0 = {
	.base = {
		.name = "uart0",
		.type_vendor = "sifive",
		.type_device = "uart0",
		.conf = uart0_conf,
	},
};

static struct __uart_device uart1 = {
	.base = {
		.name = "uart1",
		.type_vendor = "sifive",
		.type_device = "uart0",
		.conf = uart1_conf,
	},
};

static int board_hifive1_init(void)
{
	__cpu_add_device(&cpu0, __bus_get_root());
	__uart_add_device(&uart0, __bus_get_root(), 1);
	__uart_add_device(&uart1, __bus_get_root(), 1);

	__uart_set_default_console(&uart0);

	return 0;
}

define_init_func(board_hifive1_init);
