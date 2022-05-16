/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_GLIBC_LIBC_SUPPORT_H_
#define BAREMETAL_CRT_GLIBC_LIBC_SUPPORT_H_

#include <bmetal/bmetal.h>
#include <bmetal/thread.h>

#ifndef CONFIG_USE_GLIBC
#  error Include wrong libc support header.
#endif

int __init_main_thread_args(struct __thread_info *ti, int argc, char *argv[], char *envp[], char *sp);

#endif /* BAREMETAL_CRT_GLIBC_LIBC_SUPPORT_H_ */
