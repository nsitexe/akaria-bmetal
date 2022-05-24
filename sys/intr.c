/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <bmetal/intr.h>
#include <bmetal/arch.h>
#include <bmetal/printk.h>
#include <bmetal/sys/inttypes.h>

void __intr_enable_local(void)
{
	__arch_intr_enable_local();
}

void __intr_disable_local(void)
{
	__arch_intr_disable_local();
}

void __intr_restore_local(const long status)
{
	__arch_intr_restore_local(status);
}

void __intr_save_local(long *status)
{
	__arch_intr_save_local(status);
}
