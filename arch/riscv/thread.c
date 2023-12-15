
/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/thread.h>

#include <bmetal/thread.h>
#include <bmetal/arch/intr.h>

int k_arch_riscv_thread_init(struct k_thread_info *ti)
{
	k_arch_copy_current_context(&ti->regs);

	ti->regs.mstatus &= ~XSTATUS_MPP_MASK;
	ti->regs.mstatus |= XSTATUS_MPP_M;
	ti->regs.mstatus |= XSTATUS_MPIE;

	return 0;
}
