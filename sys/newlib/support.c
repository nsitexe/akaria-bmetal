/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>
#include <bmetal/syscall.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/string.h>

void _crt_start(void);

int __init_main_thread_args(struct __thread_info *ti, int argc, char *argv[], char *envp[], char *sp_user, char *sp_intr)
{
	intptr_t *v;

	sp_user = (void *)argv;
	sp_user -= sizeof(intptr_t);
	v = (intptr_t *)sp_user;
	*v = argc;

	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_1, 0);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK, (uintptr_t)sp_user);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK_INTR, (uintptr_t)sp_intr);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_INTADDR, (uintptr_t)_crt_start);

	return 0;
}

const __syscall_func_t __table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, __sys_uname),
	SYSCALL_P(SYS_getuid, __sys_getuid),
	SYSCALL_P(SYS_geteuid, __sys_geteuid),
	SYSCALL_P(SYS_getgid, __sys_getgid),
	SYSCALL_P(SYS_getegid, __sys_getegid),
	SYSCALL_P(SYS_getpid, __sys_getpid),
	SYSCALL_P(SYS_gettimeofday, __sys_gettimeofday),
	SYSCALL_P(SYS_close, __sys_close),
	SYSCALL_P(SYS_read, __sys_read),
	SYSCALL_P(SYS_write, __sys_write),
	SYSCALL_P(SYS_writev, __sys_writev),
	SYSCALL_P(SYS_brk, __sys_brk),
	SYSCALL_P(SYS_mmap, __sys_mmap),
	SYSCALL_P(SYS_munmap, __sys_munmap),
	SYSCALL_P(SYS_exit_group, __sys_exit_group),
	SYSCALL_P(SYS_exit, __sys_exit),

	/* system dependent */
	SYSCALL_P(SYS_context_switch, __sys_context_switch),
};
