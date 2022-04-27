/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_INTR_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_INTR_H_

#include <bmetal/bmetal.h>
#include <bmetal/device.h>

#define __arch_get_intr_handler    __arch_riscv_get_intr_handler
#define __arch_set_intr_handler    __arch_riscv_set_intr_handler
#define __arch_get_exc_handler    __arch_riscv_get_exc_handler
#define __arch_set_exc_handler    __arch_riscv_set_exc_handler
#define __arch_intr_enable_local     __arch_riscv_intr_enable_local
#define __arch_intr_disable_local    __arch_riscv_intr_disable_local
#define __arch_intr_restore_local    __arch_riscv_intr_restore_local
#define __arch_intr_save_local       __arch_riscv_intr_save_local

#include <bmetal/arch-gen/intr.h>

int __arch_riscv_get_intr_handler(int event, struct __event_handler **hnd);
int __arch_riscv_set_intr_handler(int event, struct __event_handler *hnd);
int __arch_riscv_get_exc_handler(int event, struct __event_handler **hnd);
int __arch_riscv_set_exc_handler(int event, struct __event_handler *hnd);
void __arch_riscv_intr_enable_local(void);
void __arch_riscv_intr_disable_local(void);
void __arch_riscv_intr_restore_local(const long status);
void __arch_riscv_intr_save_local(long *status);

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_INTR_H_ */
