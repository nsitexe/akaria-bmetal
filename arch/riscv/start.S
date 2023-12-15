/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/arch/cpu.h>
#include <bmetal/binding/intc/riscv/rv_priv.h>

.globl k_reset
.type k_reset, @function

.section ".reset", "ax"
.align 2
k_reset:
#ifdef CONFIG_RISCV_RNMI
	# Enable NMI
	csrsi CSR_MNSTATUS, MNSTATUS_NMIE
#endif /* CONFIG_RISCV_RNMI */

	# Disable global, all interrupts
	csrw mstatus, zero
	csrw mie, zero

	# Interrupt handler
	la   t0, k_trap_start
	csrw mtvec, t0

	# Global pointer
.option push
.option norelax
	la   gp, __global_pointer$
.option pop

	# Clear notify area
	la    t0, k_boot_proc
	li    t1, -1
	OP_ST t1, (t0)

	# Branch to main/sub function
	csrr t0, mhartid
	li   t1, MHARTID_MAIN
	beq  t0, t1, k_start_for_main
	j    k_start_for_sub

k_start_for_main:
	# Stack pointer
	la   sp, k_stack_idle
	li   t0, CONFIG_IDLE_STACK_SIZE
	add  sp, t0, sp
	la   t0, k_stack_intr
	li   t1, CONFIG_INTR_STACK_SIZE
	add  t0, t1, t0
	csrw mscratch, t0

	call k_start_for_common
	call k_init_system
	call k_init_leader

	# For error handling
1:
	wfi
	j   1b

k_start_for_sub:
	# Wait for notify
	csrr  t0, mhartid
1:
	# Reduce memory bus bandwidth
	nop
	nop
	nop
	nop
	fence ir, ir
	la    t1, k_boot_proc
	OP_LD t1, (t1)
	bne   t0, t1, 1b

	# Set stack/thread pointer
	la    t0, k_boot_sp_idle
	OP_LD sp, (t0)
	la    t0, k_boot_sp_intr
	OP_LD t0, (t0)
	csrw  mscratch, t0
	la    t0, k_boot_done
	li    t1, 1
	OP_ST t1, (t0)
	fence ow, ow

	call k_start_for_common
	call k_init_child

	# For error handling
1:
	wfi
	j   1b

k_start_for_common:
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
