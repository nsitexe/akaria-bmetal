/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_SYNC_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_SYNC_H_

#include <bmetal/generated/autoconf.h>

/* Instruction memory barrier */
#define imb()     __asm volatile("fence.i" : : : "memory")
/* Data memory barrier */
#define dmb()     __asm volatile("fence iorw, iorw" : : : "memory")
/* Data memory barrier for read */
#define drmb()    __asm volatile("fence ir, ir" : : : "memory")
/* Data memory barrier for write */
#define dwmb()    __asm volatile("fence ow, ow" : : : "memory")

#include <bmetal/arch-gen/sync.h>

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_SYNC_H_ */
