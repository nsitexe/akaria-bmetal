/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_NEWLIB_SYSCALL_H_
#define BAREMETAL_CRT_NEWLIB_SYSCALL_H_

#include <bmetal/generated/autoconf.h>

#ifndef CONFIG_USE_NEWLIB
#  error Include wrong syscall header.
#endif

#define MAX_SYSCALLS    10

#endif /* BAREMETAL_CRT_NEWLIB_SYSCALL_H_ */
