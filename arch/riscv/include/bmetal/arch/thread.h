/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_RISCV_ARCH_THREAD_H_
#define BAREMETAL_CRT_ARCH_RISCV_ARCH_THREAD_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

struct __thread_info;

#define __arch_thread_init        __arch_riscv_thread_init

#endif /* !__ASSEMBLER__ */


#include <bmetal/arch-gen/thread.h>


#if !defined(__ASSEMBLER__)

int __arch_riscv_thread_init(struct __thread_info *ti);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_RISCV_ARCH_THREAD_H_ */
