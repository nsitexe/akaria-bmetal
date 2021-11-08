/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <bmetal/generated/autoconf.h>

#define MSTATUS_FS    0x6000
#define MHARTID_MAIN    CONFIG_MAIN_CORE

.globl _start
.type _start, %function

.section ".text.init"
.align 2
_start:
	la   t0, trap_entry
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

	# Clear thread pointer
	li   tp, 0

	# Branch to main/sub function
	csrr t0, mhartid
	li   t1, MHARTID_MAIN
	beq  t0, t1, for_main
	j    for_sub

for_main:
	# Stack pointer
	la   sp, stack_main
	li   t0, CONFIG_MAIN_STACK_SIZE
	add  sp, t0, sp

	call _prep_main

	# For error handling
1:
	wfi
	j   1b

for_sub:
	# Set stack pointer
	csrr t0, mhartid
	addi t0, t0, 1
	li   t1, CONFIG_INTR_STACK_SIZE
	mul  sp, t0, t1

	call _prep_sub

	# For error handling
1:
	wfi
	j   1b

.align 2
trap_entry:
	# TODO: to be implemented
