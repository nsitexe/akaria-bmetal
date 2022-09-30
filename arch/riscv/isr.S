/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/asm.h>
#include <bmetal/syscall_num.h>

.globl __trap_start
.type __trap_start, @function
.globl __trap_end
.type __trap_end, @function

.section ".text", "ax"
.align 2
__trap_start:
	csrrw sp, mscratch, sp
	addi  sp, sp, -REGOFF_ALL
	store_caller_regs
	store_callee_regs
	store_special_regs
	store_status_regs t0
	csrr  t0, mscratch
	OP_ST t0, REGOFF_SP(sp)

	# Save pointer of user regs
	addi  sp, sp, -REGOFF_ALL
	store_caller_regs
	addi  a0, sp, REGOFF_ALL
	call  __cpu_set_current_user_regs
	load_caller_regs
	addi  sp, sp, REGOFF_ALL

	# Check syscall
	csrr t0, mcause
	li   t1, 0xb
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
	addi t0, sp, REGOFF_ALL
	csrw mscratch, t0
	load_status_regs t0
	load_caller_regs
	load_callee_regs
	load_special_regs
	OP_LD sp, REGOFF_SP(sp)

	mret
