/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/device.h>
#include <bmetal/init.h>

static int soc_qemu_virt_init(void)
{
	return 0;
}

define_init_func(soc_qemu_virt_init);
