/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/asm.h>
#include <bmetal/syscall_num.h>

.globl __arch_riscv_copy_current_context
.type __arch_riscv_copy_current_context, @function
.globl __arch_riscv_context_switch
.type __arch_riscv_context_switch, @function

.section ".text", "ax"
.align 2
__arch_riscv_copy_current_context:
	OP_ST sp, REGOFF_SP(a0)

	OP_CALLER_REGS(OP_ST, a0)
	OP_CALLEE_REGS(OP_ST, a0)
	OP_SPECIAL_REGS(OP_ST, a0)

	csrr  t0, mepc
	OP_ST t0, REGOFF_MEPC(a0)
	csrr  t0, mstatus
	OP_ST t0, REGOFF_MSTATUS(a0)
	OP_LD t0, REGOFF_T0(a0)

	ret
__arch_riscv_context_switch:
	li    a7, SYS_context_switch
	ecall
	ret
