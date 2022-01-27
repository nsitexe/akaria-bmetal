/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYSCALL_H_
#define BAREMETAL_CRT_SYSCALL_H_

#include <bmetal/bmetal.h>

#ifndef ASMLANG
#include <bmetal/app/bmetal/syscall.h>
#endif /* ASMLANG */

#if defined(CONFIG_USE_GLIBC)
#  include <bmetal/glibc/syscall.h>
#elif defined(CONFIG_USE_MUSL)
#  include <bmetal/musl/syscall.h>
#elif defined(CONFIG_USE_NEWLIB)
#  include <bmetal/newlib/syscall.h>
#endif

#define SYSCALL_P(num, func)    [(num)] = (__syscall_func_t)(func)

#endif /* BAREMETAL_CRT_SYSCALL_H_ */
