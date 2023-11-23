/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_TYPES_H_
#define BAREMETAL_CRT_SYS_TYPES_H_

#include <bmetal/bmetal.h>
#include <bmetal/arch/types.h>

#if !defined(__ASSEMBLER__)

typedef k_arch_clockid_t clockid_t;
typedef k_arch_mode_t mode_t;
typedef k_arch_loff_t loff_t;
typedef k_arch_off_t off_t;
typedef k_arch_rlim_t rlim_t;
typedef k_arch_ssize_t ssize_t;
typedef k_arch_suseconds_t suseconds_t;
typedef k_arch_time_t time_t;
typedef k_arch_pid_t pid_t;
typedef k_arch_uid_t uid_t;

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SYS_TYPES_H_ */
