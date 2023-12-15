/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_NEWLIB_LIBC_SUPPORT_H_
#define BAREMETAL_CRT_NEWLIB_LIBC_SUPPORT_H_

#include <bmetal/bmetal.h>
#include <bmetal/thread.h>

#ifndef CONFIG_USE_NEWLIB
#  error Include wrong libc support header.
#endif

int k_libc_init_main_thread(struct k_thread_info *ti, int argc, char *argv[], char *envp[], char *sp_user, char *sp_intr);

#endif /* BAREMETAL_CRT_NEWLIB_LIBC_SUPPORT_H_ */
