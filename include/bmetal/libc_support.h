/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_LIBC_SUPPORT_H_
#define BAREMETAL_CRT_LIBC_SUPPORT_H_

#include <bmetal/bmetal.h>

#if defined(CONFIG_USE_GLIBC)
#  include <bmetal/glibc/libc_support.h>
#elif defined(CONFIG_USE_MUSL)
#  include <bmetal/musl/libc_support.h>
#elif defined(CONFIG_USE_NEWLIB)
#  include <bmetal/newlib/libc_support.h>
#endif

void __libc_init(int argc, char *argv[], char *envp[]);

#endif /* BAREMETAL_CRT_LIBC_SUPPORT_H_ */
