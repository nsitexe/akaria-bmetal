/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <bmetal/bmetal.h>
#include <bmetal/syscall.h>

extern void __libc_init_array(void);
extern void __libc_fini_array(void);

int errno_real = 0;

int *__errno(void)
{
	return &errno_real;
}

void __libc_init(void)
{
	atexit(__libc_fini_array);
	__libc_init_array();
}

const __syscall_func_t __table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_write, __sys_write),
	SYSCALL_P(SYS_exit, __sys_exit),
};
