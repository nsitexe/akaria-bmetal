/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_

#include <bmetal/asm.h>

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
#define REGOFF_SP         (REGSIZE * 1)
#define REGOFF_GP         (REGSIZE * 2)
#define REGOFF_TP         (REGSIZE * 3)
#define REGOFF_T0         (REGSIZE * 4)
#define REGOFF_T1         (REGSIZE * 5)
#define REGOFF_T2         (REGSIZE * 6)
#define REGOFF_T3         (REGSIZE * 7)
#define REGOFF_T4         (REGSIZE * 8)
#define REGOFF_T5         (REGSIZE * 9)
#define REGOFF_T6         (REGSIZE * 10)
#define REGOFF_A0         (REGSIZE * 11)
#define REGOFF_A1         (REGSIZE * 12)
#define REGOFF_A2         (REGSIZE * 13)
#define REGOFF_A3         (REGSIZE * 14)
#define REGOFF_A4         (REGSIZE * 15)
#define REGOFF_A5         (REGSIZE * 16)
#define REGOFF_A6         (REGSIZE * 17)
#define REGOFF_A7         (REGSIZE * 18)
#define REGOFF_S0         (REGSIZE * 19)
#define REGOFF_S1         (REGSIZE * 20)
#define REGOFF_S2         (REGSIZE * 21)
#define REGOFF_S3         (REGSIZE * 22)
#define REGOFF_S4         (REGSIZE * 23)
#define REGOFF_S5         (REGSIZE * 24)
#define REGOFF_S6         (REGSIZE * 25)
#define REGOFF_S7         (REGSIZE * 26)
#define REGOFF_S8         (REGSIZE * 27)
#define REGOFF_S9         (REGSIZE * 28)
#define REGOFF_S10        (REGSIZE * 29)
#define REGOFF_S11        (REGSIZE * 30)
#define REGOFF_MEPC       (REGSIZE * 31)
#define REGOFF_MSTATUS    (REGSIZE * 32)

#define REGOFF_ALL        (REGSIZE * 33)

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

#define OP_CALLEE_REGS(OP) \
	OP s0, REGOFF_S0(sp); \
	OP s1, REGOFF_S1(sp); \
	OP s2, REGOFF_S2(sp); \
	OP s3, REGOFF_S3(sp); \
	OP s4, REGOFF_S4(sp); \
	OP s5, REGOFF_S5(sp); \
	OP s6, REGOFF_S6(sp); \
	OP s7, REGOFF_S7(sp); \
	OP s8, REGOFF_S8(sp); \
	OP s9, REGOFF_S9(sp); \
	OP s10, REGOFF_S10(sp); \
	OP s11, REGOFF_S11(sp);

#define OP_SPECIAL_REGS(OP) \
	OP gp, REGOFF_GP(sp); \
	OP tp, REGOFF_TP(sp);

.macro store_caller_regs
	OP_CALLER_REGS(OP_ST)
.endm

.macro store_callee_regs
	OP_CALLEE_REGS(OP_ST)
.endm

.macro store_special_regs
	OP_SPECIAL_REGS(OP_ST)
.endm

.macro store_status_regs tmp
	csrr  \tmp, mepc
	OP_ST \tmp, REGOFF_MEPC(sp)
	csrr  \tmp, mstatus
	OP_ST \tmp, REGOFF_MSTATUS(sp)
.endm

.macro load_caller_regs
	OP_CALLER_REGS(OP_LD)
.endm

.macro load_callee_regs
	OP_CALLEE_REGS(OP_LD)
.endm

.macro load_special_regs
	OP_SPECIAL_REGS(OP_LD)
.endm

.macro load_status_regs tmp
	OP_LD \tmp, REGOFF_MEPC(sp)
	csrw  mepc, \tmp
	OP_LD \tmp, REGOFF_MSTATUS(sp)
	csrw  mstatus, \tmp
.endm

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_ */
