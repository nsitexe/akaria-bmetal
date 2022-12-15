/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/cpu.h>
#include <bmetal/syscall.h>

.globl __trap_start
.type __trap_start, @function
.globl __trap_end
.type __trap_end, @function

#define XCAUSE_SYSCALL_M    0xb
#define XSTATUS_MIE         BIT(3)

.section ".text", "ax"
.align 2
__trap_start:
	csrrw sp, mscratch, sp
	beq   sp, x0, __trap_in_kernel

__trap_in_user:
	addi  sp, sp, -REGOFF_ALL
	store_caller_regs
	store_callee_regs
	store_special_regs
	store_status_regs t0
	csrr  t0, mscratch
	OP_ST t0, REGOFF_SP(sp)
	addi  t0, sp, REGOFF_ALL
	OP_ST t0, REGOFF_MSCRATCH(sp)
	csrw  mscratch, x0
	j     __trap_common

__trap_in_kernel:
	csrrw sp, mscratch, sp
	addi  sp, sp, -REGOFF_ALL
	store_caller_regs
	store_callee_regs
	store_special_regs
	store_status_regs t0
	addi  t0, sp, REGOFF_ALL
	OP_ST t0, REGOFF_SP(sp)
	j     __trap_common

__trap_common:
	# Save pointer of user regs
	# This may break caller-saved regs
	mv    a0, sp
	call  __cpu_set_current_user_regs
	OP_LD a0, REGOFF_A0(sp)
	OP_LD a1, REGOFF_A1(sp)
	OP_LD a2, REGOFF_A2(sp)
	OP_LD a3, REGOFF_A3(sp)
	OP_LD a4, REGOFF_A4(sp)
	OP_LD a5, REGOFF_A5(sp)
	OP_LD a6, REGOFF_A6(sp)
	OP_LD a7, REGOFF_A7(sp)

	# Check syscall
	csrr t0, mcause
	li   t1, XCAUSE_SYSCALL_M
	beq  t0, t1, __trap_syscall

	# Check interrupt (mcause MSB = 1) or exception (MSB = 0)
	not  t1, x0
	srli t1, t1, 1
	not  t1, t1
	and  t0, t0, t1
	bne  x0, t0, __trap_interrupt
	j    __trap_exception

__trap_syscall:
	# Return to next instruction of ecall
	csrr  t0, mepc
	addi  t0, t0, 4
	OP_ST t0, REGOFF_MEPC(sp)

	# Check syscall number
	blt   a7, zero, __trap_syscall_unknown
	li    t0, MAX_SYSCALLS
	bge   a7, t0, __trap_syscall_unknown

	# Find syscall handler
	la    t0, __table_syscalls
	slli  t1, a7, REGOFF
	add   t0, t0, t1
	OP_LD t0, (t0)
	beqz  t0, __trap_syscall_unknown
	jalr  t0

__trap_syscall_end:
	# Store return value of syscall
	OP_ST a0, REGOFF_A0(sp)

	j     __trap_end

__trap_syscall_unknown:
	mv    a6, a5
	mv    a5, a4
	mv    a4, a3
	mv    a3, a2
	mv    a2, a1
	mv    a1, a0
	mv    a0, a7
	call  __sys_unknown
	j     __trap_syscall_end

__trap_interrupt:
	csrr a0, mcause
	call __arch_riscv_interrupt
	j    __trap_end

__trap_exception:
	csrr a0, mcause
	call __arch_riscv_exception
	j    __trap_end

__trap_end:
	# Disable interrupt
	li   t0, XSTATUS_MIE
	csrc mstatus, t0

	load_status_regs t0
	load_caller_regs
	load_callee_regs
	load_special_regs
	OP_LD sp, REGOFF_SP(sp)

	mret
