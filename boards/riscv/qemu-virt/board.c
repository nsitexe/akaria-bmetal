/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>
#include <bmetal/uart.h>

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
	__uart_add_device(&uart0, __bus_get_root(), 1);

	return 0;
}

define_init_func(board_qemu_virt_init);
