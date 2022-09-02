/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stddef.h>

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>
#include <bmetal/syscall.h>
#include <bmetal/thread.h>

void __libc_init_array(void);
void __libc_fini_array(void);
int atexit(void (*function)(void));
void exit(int status);
int main(int argc, char *argv[], char *envp[]);

int errno_real = 0;

int *__errno(void)
{
	return &errno_real;
}

static void __libc_init(int argc, char *argv[], char *envp[])
{
	atexit(__libc_fini_array);
	__libc_init_array();

	int r = main(argc, argv, envp);

	exit(r);
}

int __init_main_thread_args(struct __thread_info *ti, int argc, char *argv[], char *envp[], char *sp)
{
	sp = (void *)argv;

	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_1, argc);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_2, (uintptr_t)argv);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_3, (uintptr_t)envp);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK, (uintptr_t)sp);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_INTADDR, (uintptr_t)__libc_init);

	return 0;
}

const __syscall_func_t __table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, __sys_uname),
	SYSCALL_P(SYS_getuid, __sys_getuid),
	SYSCALL_P(SYS_geteuid, __sys_geteuid),
	SYSCALL_P(SYS_close, __sys_close),
	SYSCALL_P(SYS_write, __sys_write),
	SYSCALL_P(SYS_writev, __sys_writev),
	SYSCALL_P(SYS_exit_group, __sys_exit_group),
	SYSCALL_P(SYS_exit, __sys_exit),
	SYSCALL_P(SYS_brk, __sys_brk),
	SYSCALL_P(SYS_mmap, __sys_mmap),
	SYSCALL_P(SYS_munmap, __sys_munmap),

	/* system dependent */
	SYSCALL_P(SYS_context_switch, __sys_context_switch),
};
