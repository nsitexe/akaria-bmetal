/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_LIBC_SUPPORT_H_
#define BAREMETAL_CRT_LIBC_SUPPORT_H_

#include <bmetal/bmetal.h>

#if defined(CONFIG_USE_LIBC)
#  include <bmetal/libc/libc_support.h>
#elif defined(CONFIG_USE_NEWLIB)
#  include <bmetal/newlib/libc_support.h>
#endif

#endif /* BAREMETAL_CRT_LIBC_SUPPORT_H_ */
