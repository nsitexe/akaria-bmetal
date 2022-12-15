/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/cpu.h>
#include <bmetal/bindings/intc/riscv/rv_priv.h>

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
	la   sp, __stack_idle
	li   t0, CONFIG_IDLE_STACK_SIZE
	add  sp, t0, sp
	la   t0, __stack_intr
	li   t1, CONFIG_INTR_STACK_SIZE
	add  t0, t1, t0
	csrw mscratch, t0

	call __start_for_common
	call __init_system
	call __init_leader

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
	fence ir, ir
	la    t1, __boot_proc
	OP_LD t1, (t1)
	bne   t0, t1, 1b

	# Set stack/thread pointer
	la    t0, __boot_sp_idle
	OP_LD sp, (t0)
	la    t0, __boot_sp_intr
	OP_LD t0, (t0)
	csrw  mscratch, t0
	la    t0, __boot_done
	li    t1, 1
	OP_ST t1, (t0)
	fence ow, ow

	call __start_for_common
	call __init_child

	# For error handling
1:
	wfi
	j   1b

__start_for_common:
	# Enable FPU
#ifdef CONFIG_FPU
	li   t0, RV_STATUS_FS_DIRTY
	csrs mstatus, t0
	csrw fcsr, x0
#endif
#ifdef CONFIG_VPU
	li   t0, RV_STATUS_VS_DIRTY
	csrs mstatus, t0
#endif
	ret
