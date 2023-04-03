
/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/thread.h>

#include <bmetal/thread.h>
#include <bmetal/arch/intr.h>

int __arch_riscv_thread_init(struct __thread_info *ti)
{
	__arch_copy_current_context(&ti->regs);

	ti->regs.mstatus &= ~XSTATUS_MPP_MASK;
	ti->regs.mstatus |= XSTATUS_MPP_M;
	ti->regs.mstatus |= XSTATUS_MPIE;

	return 0;
}
