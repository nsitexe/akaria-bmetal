/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYSCALL_H_
#define BAREMETAL_CRT_SYSCALL_H_

#include <stdint.h>
#include <unistd.h>

#include <bmetal/bmetal.h>
#include <bmetal/syscall_num.h>

#define SYSCALL_P(num, func)    [(num)] = (__syscall_func_t)(func)

#define __NEW_UTS_LEN 64

struct new_utsname {
	char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
};

typedef intptr_t (*__syscall_func_t)(intptr_t no, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

intptr_t __sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

int __sys_uname(struct new_utsname *name);
int __sys_close(int fd);
ssize_t __sys_write(int fd, const void *buf, size_t count);
void __sys_exit(int status);
void *__sys_brk(void *addr);

#endif /* BAREMETAL_CRT_SYSCALL_H_ */
