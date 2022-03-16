/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/asm.h>
#include <bmetal/syscall_num.h>

.globl __start_trap
.type __start_trap, @function

.globl __sys_unknown
.type __sys_unknown, @function
.globl __arch_riscv_interrupt
.type __arch_riscv_interrupt, @function
.globl __arch_riscv_exception
.type __arch_riscv_exception, @function

.section ".text", "ax"
.align 2
__start_trap:
	store_caller_regs

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
	# Check syscall number
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

	# Return to next instruction of ecall
	csrr  t0, mepc
	addi  t0, t0, 4
	csrw  mepc, t0
	j     __end_trap

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
	j    __end_trap

__trap_exception:
	csrr a0, mcause
	slli a0, a0, 1
	srli a0, a0, 1
	call __arch_riscv_exception
	j    __end_trap

__end_trap:
	load_caller_regs
	mret
