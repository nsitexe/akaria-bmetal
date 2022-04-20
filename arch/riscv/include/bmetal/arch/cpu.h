/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_CPU_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_CPU_H_

#include <stdint.h>

#include <bmetal/bmetal.h>

#define __arch_get_cpu_id    __arch_riscv_get_cpu_id
int __arch_riscv_get_cpu_id(void);

#define __arch_user_regs_t    __arch_riscv_user_regs_t
typedef struct __arch_riscv_user_regs {
	uintptr_t ra;
	uintptr_t sp;
	uintptr_t gp;
	uintptr_t tp;
	uintptr_t t0;
	uintptr_t t1;
	uintptr_t t2;
	uintptr_t t3;
	uintptr_t t4;
	uintptr_t t5;
	uintptr_t t6;
	uintptr_t a0;
	uintptr_t a1;
	uintptr_t a2;
	uintptr_t a3;
	uintptr_t a4;
	uintptr_t a5;
	uintptr_t a6;
	uintptr_t a7;
	uintptr_t s0;
	uintptr_t s1;
	uintptr_t s2;
	uintptr_t s3;
	uintptr_t s4;
	uintptr_t s5;
	uintptr_t s6;
	uintptr_t s7;
	uintptr_t s8;
	uintptr_t s9;
	uintptr_t s10;
	uintptr_t s11;
	uintptr_t mepc;
	uintptr_t mstatus;
} __arch_riscv_user_regs_t;

#include <bmetal/arch-gen/cpu.h>

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_CPU_H_ */
