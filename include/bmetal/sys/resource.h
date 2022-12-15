/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_RESOURCE_H_
#define BAREMETAL_CRT_SYS_RESOURCE_H_

#include <bmetal/bmetal.h>
#include <bmetal/sys/types.h>

#define RLIMIT_CPU        0
#define RLIMIT_FSIZE      1
#define RLIMIT_DATA       2
#define RLIMIT_STACK      3
#define RLIMIT_CORE       4

#if !defined(__ASSEMBLER__)

struct rlimit {
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

struct rlimit64 {
	uint64_t rlim_cur;
	uint64_t rlim_max;
};

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SYS_RESOURCE_H_ */
