/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/generated/autoconf.h>
#include <bmetal/syscall.h>

#define MSTATUS_FS    0x6000
#define MHARTID_MAIN    CONFIG_MAIN_CORE

#ifdef CONFIG_64BIT
#  define REGSIZE     8
#  define REGOFF      3
#  define OP_LD       ld
#  define OP_ST       sd
#else /* CONFIG_64BIT */
#  define REGSIZE     4
#  define REGOFF      2
#  define OP_LD       lw
#  define OP_ST       sw
#endif /* CONFIG_64BIT */

/* caller-saved registers */
#define REGOFF_RA         (REGSIZE * 0)
/* sp is not saved */
/* gp is not saved */
#define REGOFF_TP         (REGSIZE * 1)
#define REGOFF_T0         (REGSIZE * 2)
#define REGOFF_T1         (REGSIZE * 3)
#define REGOFF_T2         (REGSIZE * 4)
#define REGOFF_T3         (REGSIZE * 5)
#define REGOFF_T4         (REGSIZE * 6)
#define REGOFF_T5         (REGSIZE * 7)
#define REGOFF_T6         (REGSIZE * 8)
#define REGOFF_A0         (REGSIZE * 9)
#define REGOFF_A1         (REGSIZE * 10)
#define REGOFF_A2         (REGSIZE * 11)
#define REGOFF_A3         (REGSIZE * 12)
#define REGOFF_A4         (REGSIZE * 13)
#define REGOFF_A5         (REGSIZE * 14)
#define REGOFF_A6         (REGSIZE * 15)
#define REGOFF_A7         (REGSIZE * 16)
#define REGOFF_MEPC       (REGSIZE * 17)
#define REGOFF_MSTATUS    (REGSIZE * 18)

#define REGOFF_ALL        (REGSIZE * 18)

#define OP_CALLER_REGS(OP) \
	OP ra, REGOFF_RA(sp); \
	OP t0, REGOFF_T0(sp); \
	OP t1, REGOFF_T1(sp); \
	OP t2, REGOFF_T2(sp); \
	OP t3, REGOFF_T3(sp); \
	OP t4, REGOFF_T4(sp); \
	OP t5, REGOFF_T5(sp); \
	OP t6, REGOFF_T6(sp); \
	OP a0, REGOFF_A0(sp); \
	OP a1, REGOFF_A1(sp); \
	OP a2, REGOFF_A2(sp); \
	OP a3, REGOFF_A3(sp); \
	OP a4, REGOFF_A4(sp); \
	OP a5, REGOFF_A5(sp); \
	OP a6, REGOFF_A6(sp); \
	OP a7, REGOFF_A7(sp);

.globl _start
.type _start, @function
.globl __prep_main
.type __prep_main, @function
.globl __prep_sub
.type __prep_sub, @function

.globl __sys_unknown
.type __sys_unknown, @function

.macro store_caller_regs
	addi sp, sp, -REGOFF_ALL
	OP_CALLER_REGS(OP_ST)
.endm

.macro load_caller_regs
	OP_CALLER_REGS(OP_LD)
	addi sp, sp, REGOFF_ALL
.endm

.section ".reset", "ax"
.align 2
_start:
	# Interrupt handler
	la   t0, __start_trap
	csrw mtvec, t0

	# Global pointer
.option push
.option norelax
	la   gp, __global_pointer$
.option pop

	# Enable FPU
#ifdef CONFIG_FPU
	li   t0, MSTATUS_FS
	csrs mstatus, t0
	csrw fcsr, x0
#endif

	# Clear notify area
	la    t0, __boot_proc
	li    t1, -1
	OP_ST t1, (t0)

	# Branch to main/sub function
	csrr t0, mhartid
	li   t1, MHARTID_MAIN
	beq  t0, t1, for_main
	j    for_sub

for_main:
	# Stack pointer
	la   sp, __stack_intr
	li   t0, CONFIG_INTR_STACK_SIZE
	add  sp, t0, sp

	call __prep_main

	# For error handling
1:
	wfi
	j   1b

for_sub:
	# Wait for notify
	csrr  t0, mhartid
	la    t1, __boot_proc
	OP_LD t1, (t1)
	bne   t0, t1, for_sub

	# Set stack/thread pointer
	la    t0, __boot_sp
	OP_LD sp, (t0)
	la    t0, __boot_done
	li    t1, 1
	OP_ST t1, (t0)
	fence iorw, iorw

	call __prep_sub

	# For error handling
1:
	wfi
	j   1b

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
	# TODO: to be implemented
1:
	wfi
	j 1b

__trap_exception:
	# TODO: to be implemented
1:
	wfi
	j 1b

__end_trap:
	load_caller_regs
	mret
