/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_

#include <bmetal/bmetal.h>

#define MSTATUS_VS    0x0600
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
#define REGOFF_MSCRATCH   (REGSIZE * 33)

#define REGOFF_ALL        (REGSIZE * 34)

#define OP_CALLER_REGS(OP, pt) \
	OP ra, REGOFF_RA(pt); \
	OP t0, REGOFF_T0(pt); \
	OP t1, REGOFF_T1(pt); \
	OP t2, REGOFF_T2(pt); \
	OP t3, REGOFF_T3(pt); \
	OP t4, REGOFF_T4(pt); \
	OP t5, REGOFF_T5(pt); \
	OP t6, REGOFF_T6(pt); \
	OP a0, REGOFF_A0(pt); \
	OP a1, REGOFF_A1(pt); \
	OP a2, REGOFF_A2(pt); \
	OP a3, REGOFF_A3(pt); \
	OP a4, REGOFF_A4(pt); \
	OP a5, REGOFF_A5(pt); \
	OP a6, REGOFF_A6(pt); \
	OP a7, REGOFF_A7(pt);

#define OP_CALLEE_REGS(OP, pt) \
	OP s0, REGOFF_S0(pt); \
	OP s1, REGOFF_S1(pt); \
	OP s2, REGOFF_S2(pt); \
	OP s3, REGOFF_S3(pt); \
	OP s4, REGOFF_S4(pt); \
	OP s5, REGOFF_S5(pt); \
	OP s6, REGOFF_S6(pt); \
	OP s7, REGOFF_S7(pt); \
	OP s8, REGOFF_S8(pt); \
	OP s9, REGOFF_S9(pt); \
	OP s10, REGOFF_S10(pt); \
	OP s11, REGOFF_S11(pt);

#define OP_SPECIAL_REGS(OP, pt) \
	OP gp, REGOFF_GP(pt); \
	OP tp, REGOFF_TP(pt);

.macro store_caller_regs
	OP_CALLER_REGS(OP_ST, sp)
.endm

.macro store_callee_regs
	OP_CALLEE_REGS(OP_ST, sp)
.endm

.macro store_special_regs
	OP_SPECIAL_REGS(OP_ST, sp)
.endm

.macro store_status_regs tmp
	csrr  \tmp, mepc
	OP_ST \tmp, REGOFF_MEPC(sp)
	csrr  \tmp, mstatus
	OP_ST \tmp, REGOFF_MSTATUS(sp)
	csrr  \tmp, mscratch
	OP_ST \tmp, REGOFF_MSCRATCH(sp)
.endm

.macro load_caller_regs
	OP_CALLER_REGS(OP_LD, sp)
.endm

.macro load_callee_regs
	OP_CALLEE_REGS(OP_LD, sp)
.endm

.macro load_special_regs
	OP_SPECIAL_REGS(OP_LD, sp)
.endm

.macro load_status_regs tmp
	OP_LD \tmp, REGOFF_MEPC(sp)
	csrw  mepc, \tmp
	OP_LD \tmp, REGOFF_MSTATUS(sp)
	csrw  mstatus, \tmp
	OP_LD \tmp, REGOFF_MSCRATCH(sp)
	csrw  mscratch, \tmp
.endm

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_ASM_H_ */
