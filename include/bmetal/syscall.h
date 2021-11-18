/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYSCALL_H_
#define BAREMETAL_CRT_SYSCALL_H_

#include <bmetal/bmetal.h>

#ifndef ASMLANG
#include <bmetal/app/bmetal/syscall.h>
#endif /* ASMLANG */

#if defined(CONFIG_USE_MUSL)
#  include <bmetal/musl/syscall.h>
#elif defined(CONFIG_USE_NEWLIB)
#  include <bmetal/newlib/syscall.h>
#endif

#endif /* BAREMETAL_CRT_SYSCALL_H_ */
