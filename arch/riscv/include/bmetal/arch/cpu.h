/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_CPU_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_CPU_H_

#include <bmetal/bmetal.h>

#define __arch_get_cpu_id    __arch_riscv_get_cpu_id
int __arch_riscv_get_cpu_id(void);

#include <bmetal/arch-gen/cpu.h>

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_CPU_H_ */
