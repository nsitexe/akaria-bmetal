/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <bmetal/bmetal.h>
#include <bmetal/syscall.h>

void __libc_init_array(void);
void __libc_fini_array(void);
int main(int argc, char *argv[], char *envp[]);

int errno_real = 0;

int *__errno(void)
{
	return &errno_real;
}

void __libc_init(int argc, char *argv[], char *envp[])
{
	atexit(__libc_fini_array);
	__libc_init_array();

	int r = main(argc, argv, envp);

	exit(r);
}

const __syscall_func_t __table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, __sys_uname),
	SYSCALL_P(SYS_close, __sys_close),
	SYSCALL_P(SYS_write, __sys_write),
	SYSCALL_P(SYS_exit, __sys_exit),
	SYSCALL_P(SYS_brk, __sys_brk),
	SYSCALL_P(SYS_mmap, __sys_mmap),
	SYSCALL_P(SYS_munmap, __sys_munmap),

	/* system dependent */
	SYSCALL_P(SYS_context_switch, __sys_context_switch),
};
