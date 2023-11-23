/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_THREAD_H_
#define BAREMETAL_CRT_ARCH_GEN_THREAD_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

#ifndef k_arch_thread_init
#  error Please implement k_arch_thread_init() function.
#endif

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_GEN_THREAD_H_ */
