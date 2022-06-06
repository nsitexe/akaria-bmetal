/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/asm.h>

.globl __reset
.type __reset, @function

.section ".reset", "ax"
.align 2
__reset:
	# Disable global, all interrupts
	csrw mstatus, zero
	csrw mie, zero

	# Interrupt handler
	la   t0, __trap_start
	csrw mtvec, t0

	# Global pointer
.option push
.option norelax
	la   gp, __global_pointer$
.option pop

	# Clear notify area
	la    t0, __boot_proc
	li    t1, -1
	OP_ST t1, (t0)

	# Branch to main/sub function
	csrr t0, mhartid
	li   t1, MHARTID_MAIN
	beq  t0, t1, __start_for_main
	j    __start_for_sub

__start_for_main:
	# Stack pointer
	la   sp, __stack_intr
	li   t0, CONFIG_INTR_STACK_SIZE
	add  sp, t0, sp

	call __start_for_common
	call __prep_main

	# For error handling
1:
	wfi
	j   1b

__start_for_sub:
	# Wait for notify
	csrr  t0, mhartid
1:
	# Reduce memory bus bandwidth
	nop
	nop
	nop
	nop
	la    t1, __boot_proc
	OP_LD t1, (t1)
	bne   t0, t1, 1b

	# Set stack/thread pointer
	la    t0, __boot_sp
	OP_LD sp, (t0)
	la    t0, __boot_done
	li    t1, 1
	OP_ST t1, (t0)
	fence iorw, iorw

	call __start_for_common
	call __prep_sub

	# For error handling
1:
	wfi
	j   1b

__start_for_common:
	# Enable FPU
#ifdef CONFIG_FPU
	li   t0, MSTATUS_FS
	csrs mstatus, t0
	csrw fcsr, x0
#endif
	ret
