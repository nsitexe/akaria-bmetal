/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_SYNC_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_SYNC_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

/* Instruction memory barrier */
#define imb()     __asm volatile ("fence.i" : : : "memory")
/* Data memory barrier */
#define dmb()     __asm volatile ("fence iorw, iorw" : : : "memory")
/* Data memory barrier for read */
#define drmb()    __asm volatile ("fence ir, ir" : : : "memory")
/* Data memory barrier for write */
#define dwmb()    __asm volatile ("fence ow, ow" : : : "memory")
/* No operation */
#define noop()    __asm volatile ("nop")

#endif /* !__ASSEMBLER__ */


#include <bmetal/arch-gen/sync.h>

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_SYNC_H_ */
