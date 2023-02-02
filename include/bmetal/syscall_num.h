/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYSCALL_NUM_H_
#define BAREMETAL_CRT_SYSCALL_NUM_H_

#if defined(CONFIG_USE_LIBC)
#  include <bmetal/libc/syscall_num.h>
#elif defined(CONFIG_USE_NEWLIB)
#  include <bmetal/newlib/syscall_num.h>
#endif

#endif /* BAREMETAL_CRT_SYSCALL_NUM_H_ */
