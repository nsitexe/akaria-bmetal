/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_LIBC_LIBC_SUPPORT_H_
#define BAREMETAL_CRT_LIBC_LIBC_SUPPORT_H_

#ifndef CONFIG_USE_LIBC
#  error Include wrong libc support header.
#endif

#include <bmetal/bmetal.h>
#include <bmetal/thread.h>

int k_libc_init_main_thread(struct k_thread_info *ti, int argc, char *argv[], char *envp[], char *sp_user, char *sp_intr);

#endif /* BAREMETAL_CRT_LIBC_LIBC_SUPPORT_H_ */
