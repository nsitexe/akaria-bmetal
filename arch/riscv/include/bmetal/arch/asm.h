/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_

#include <bmetal/generated/autoconf.h>

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

.macro store_caller_regs
	addi sp, sp, -REGOFF_ALL
	OP_CALLER_REGS(OP_ST)
.endm

.macro load_caller_regs
	OP_CALLER_REGS(OP_LD)
	addi sp, sp, REGOFF_ALL
.endm

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_ */
