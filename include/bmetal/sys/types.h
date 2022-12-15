/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_TYPES_H_
#define BAREMETAL_CRT_SYS_TYPES_H_

#include <bmetal/bmetal.h>
#include <bmetal/arch/types.h>

#if !defined(__ASSEMBLER__)

typedef __arch_clockid_t clockid_t;
typedef __arch_loff_t loff_t;
typedef __arch_off_t off_t;
typedef __arch_rlim_t rlim_t;
typedef __arch_ssize_t ssize_t;
typedef __arch_suseconds_t suseconds_t;
typedef __arch_time_t time_t;
typedef __arch_pid_t pid_t;
typedef __arch_uid_t uid_t;

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SYS_TYPES_H_ */
