/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_BINDINGS_INTC_RISCV_PRIV_H_
#define BAREMETAL_CRT_BINDINGS_INTC_RISCV_PRIV_H_

#include <bmetal/bindings/bindings.h>

/* RISC-V Privileged Architecture Version 1.10 */

/* mstatus, sstatus */
#define RV_STATUS_UIE     0
#define RV_STATUS_SIE     1
#define RV_STATUS_MIE     3
#define RV_STATUS_UPIE    4
#define RV_STATUS_SPIE    5
#define RV_STATUS_MPIE    7
#define RV_STATUS_SPP     8
#define RV_STATUS_MPRV    17
#define RV_STATUS_SUM     18
#define RV_STATUS_MXR     19
#define RV_STATUS_TVM     20
#define RV_STATUS_TW      21
#define RV_STATUS_TSR     22

/* mie, mip, sie, sip */
/* User software interrupt enable/pending */
#define RV_IX_USIX       0
/* Supervisor software interrupt enable/pending */
#define RV_IX_SSIX       1
/* Machine software interrupt enable/pending */
#define RV_IX_MSIX       3
/* User timer interrupt enable/pending */
#define RV_IX_UTIX       4
/* Supervisor timer interrupt enable/pending */
#define RV_IX_STIX       5
/* Machine timer interrupt enable/pending */
#define RV_IX_MTIX       7
/* User external interrupt enable/pending */
#define RV_IX_UEIX       8
/* Supervisor external interrupt enable/pending */
#define RV_IX_SEIX       9
/* Machine external interrupt enable/pending */
#define RV_IX_MEIX       11

/* mcause, scause */
/* User software interrupt */
#define RV_CAUSE_INT_U_SW              0
/* Supervisor software interrupt */
#define RV_CAUSE_INT_S_SW              1
/* Machine software interrupt */
#define RV_CAUSE_INT_M_SW              3
/* User timer interrupt */
#define RV_CAUSE_INT_U_TM              4
/* Supervisor timer interrupt */
#define RV_CAUSE_INT_S_TM              5
/* Machine timer interrupt */
#define RV_CAUSE_INT_M_TM              7
/* User external interrupt */
#define RV_CAUSE_INT_U_EX              8
/* Supervisor external interrupt */
#define RV_CAUSE_INT_S_EX              9
/* Machine external interrupt */
#define RV_CAUSE_INT_M_EX              11

#define RV_CAUSE_INT_MAX_NUM           12

/* Instruction address misaligned */
#define RV_CAUSE_EXC_INS_ADDR          0
/* Instruction access fault */
#define RV_CAUSE_EXC_INS_ACC           1
/* Illegal instruction */
#define RV_CAUSE_EXC_ILL               2
/* Breakpoint */
#define RV_CAUSE_EXC_BRK               3
/* Load address misaligned */
#define RV_CAUSE_EXC_LD_ADDR           4
/* Load access fault */
#define RV_CAUSE_EXC_LD_FAULT          5
/* Store/AMO address misaligned */
#define RV_CAUSE_EXC_ST_ADDR           6
/* Store/AMO access fault */
#define RV_CAUSE_EXC_ST_FAULT          7
/* Environment call from U-mode */
#define RV_CAUSE_EXC_ECALL_U           8
/* Environment call from S-mode */
#define RV_CAUSE_EXC_ECALL_S           9
/* Environment call from M-mode */
#define RV_CAUSE_EXC_ECALL_M           11
/* Instruction page fault */
#define RV_CAUSE_EXC_INS_PAGE_FAULT    12
/* Load page fault */
#define RV_CAUSE_EXC_LD_PAGE_FAULT     13
/* Store/AMO page fault */
#define RV_CAUSE_EXC_ST_PAGE_FAULT     15

#define RV_CAUSE_EXC_MAX_NUM           16

// TODO: change to CONFIG or not?
#define USE_M_MODE    1

#ifdef USE_M_MODE
#define RV_STATUS_IE       RV_STATUS_MIE
#define RV_STATUS_PIE      RV_STATUS_MPIE
#define RV_IX_SIX          RV_IX_MSIX
#define RV_IX_TIX          RV_IX_MTIX
#define RV_IX_EIX          RV_IX_MEIX
#define RV_CAUSE_INT_SW    RV_CAUSE_INT_M_SW
#define RV_CAUSE_INT_TM    RV_CAUSE_INT_M_TM
#define RV_CAUSE_INT_EX    RV_CAUSE_INT_M_EX
#else
#define RV_STATUS_IE       RV_STATUS_SIE
#define RV_STATUS_PIE      RV_STATUS_SPIE
#define RV_IX_SIX          RV_IX_SSIX
#define RV_IX_TIX          RV_IX_STIX
#define RV_IX_EIX          RV_IX_SEIX
#define RV_CAUSE_INT_SW    RV_CAUSE_INT_S_SW
#define RV_CAUSE_INT_TM    RV_CAUSE_INT_S_TM
#define RV_CAUSE_INT_EX    RV_CAUSE_INT_S_EX
#endif

#endif /* BAREMETAL_CRT_BINDINGS_INTC_RISCV_PRIV_H_ */
