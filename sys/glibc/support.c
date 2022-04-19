/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include <bmetal/bmetal.h>
#include <bmetal/syscall.h>

void __libc_start_main(void *main, int argc, char **argv, void *init, void *fini, void *rtld_fini, void *stack_end);
int main(int argc, char *argv[], char *envp[]);

void __libc_init(int argc, char *argv[], char *envp[])
{
	__libc_start_main(main, argc, argv, NULL,
		NULL, NULL, (void *)((uintptr_t)&argc & ~0xfff));
}

const __syscall_func_t __table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, __sys_uname),
	SYSCALL_P(SYS_close, __sys_close),
	SYSCALL_P(SYS_write, __sys_write),
	SYSCALL_P(SYS_exit, __sys_exit),
	SYSCALL_P(SYS_brk, __sys_brk),
	SYSCALL_P(SYS_mmap, __sys_mmap),
	SYSCALL_P(SYS_munmap, __sys_munmap),
	SYSCALL_P(SYS_mprotect, __sys_mprotect),
};
