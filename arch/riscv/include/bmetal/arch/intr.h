/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_INTR_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_INTR_H_

#include <bmetal/bmetal.h>
#include <bmetal/device.h>
#include <bmetal/event.h>
#include <bmetal/bindings/intc/riscv/rv_priv.h>

#if !defined(__ASSEMBLER__)

/* mstatus, sstatus */
#define XSTATUS_UIE         BIT(RV_STATUS_UIE )
#define XSTATUS_SIE         BIT(RV_STATUS_SIE )
#define XSTATUS_MIE         BIT(RV_STATUS_MIE )
#define XSTATUS_UPIE        BIT(RV_STATUS_UPIE)
#define XSTATUS_SPIE        BIT(RV_STATUS_SPIE)
#define XSTATUS_MPIE        BIT(RV_STATUS_MPIE)
#define XSTATUS_SPP         BIT(RV_STATUS_SPP )
#define XSTATUS_MPP_U       (0x0 << RV_STATUS_MPP)
#define XSTATUS_MPP_S       (0x1 << RV_STATUS_MPP)
#define XSTATUS_MPP_M       (0x3 << RV_STATUS_MPP)
#define XSTATUS_MPP_MASK    (0x3 << RV_STATUS_MPP)
#define XSTATUS_FS_MASK     (0x3 << RV_STATUS_FS)
#define XSTATUS_XS_MASK     (0x3 << RV_STATUS_XS)
#define XSTATUS_MPRV        BIT(RV_STATUS_MPRV)
#define XSTATUS_SUM         BIT(RV_STATUS_SUM )
#define XSTATUS_MXR         BIT(RV_STATUS_MXR )
#define XSTATUS_TVM         BIT(RV_STATUS_TVM )
#define XSTATUS_TW          BIT(RV_STATUS_TW  )
#define XSTATUS_TSR         BIT(RV_STATUS_TSR )

#define XSTATUS_IE          BIT(RV_STATUS_IE)
#define XSTATUS_PIE         BIT(RV_STATUS_PIE)

/* mie, mip, sie, sip */
#define XIX_USIX        BIT(RV_IX_USIX)
#define XIX_SSIX        BIT(RV_IX_SSIX)
#define XIX_MSIX        BIT(RV_IX_MSIX)
#define XIX_UTIX        BIT(RV_IX_UTIX)
#define XIX_STIX        BIT(RV_IX_STIX)
#define XIX_MTIX        BIT(RV_IX_MTIX)
#define XIX_UEIX        BIT(RV_IX_UEIX)
#define XIX_SEIX        BIT(RV_IX_SEIX)
#define XIX_MEIX        BIT(RV_IX_MEIX)

#define k_arch_get_intr_handler      k_arch_riscv_get_intr_handler
#define k_arch_set_intr_handler      k_arch_riscv_set_intr_handler
#define k_arch_get_exc_handler       k_arch_riscv_get_exc_handler
#define k_arch_set_exc_handler       k_arch_riscv_set_exc_handler
#define k_arch_intr_enable_local     k_arch_riscv_intr_enable_local
#define k_arch_intr_disable_local    k_arch_riscv_intr_disable_local
#define k_arch_intr_restore_local    k_arch_riscv_intr_restore_local
#define k_arch_intr_save_local       k_arch_riscv_intr_save_local

#endif /* !__ASSEMBLER__ */


#include <bmetal/arch-gen/intr.h>


#if !defined(__ASSEMBLER__)

int k_arch_riscv_get_intr_handler(int event, struct k_event_handler **hnd);
int k_arch_riscv_set_intr_handler(int event, struct k_event_handler *hnd);
int k_arch_riscv_get_exc_handler(int event, struct k_event_handler **hnd);
int k_arch_riscv_set_exc_handler(int event, struct k_event_handler *hnd);
void k_arch_riscv_intr_enable_local(void);
void k_arch_riscv_intr_disable_local(void);
void k_arch_riscv_intr_restore_local(const long status);
void k_arch_riscv_intr_save_local(long *status);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_INTR_H_ */
