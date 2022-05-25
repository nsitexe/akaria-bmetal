/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stddef.h>

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>
#include <bmetal/syscall.h>
#include <bmetal/thread.h>
#include <bmetal/sys/string.h>

void _start(void);

int __init_main_thread_args(struct __thread_info *ti, int argc, char *argv[], char *envp[], char *sp)
{
	uintptr_t v;

	sp = (void *)argv;
	sp -= sizeof(uintptr_t);
	v = argc;
	kmemcpy(sp, &v, sizeof(uintptr_t));

	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_1, (uintptr_t)NULL);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK, (uintptr_t)sp);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_INTADDR, (uintptr_t)_start);

	return 0;
}

const __syscall_func_t __table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, __sys_uname),
	SYSCALL_P(SYS_close, __sys_close),
	SYSCALL_P(SYS_write, __sys_write),
	SYSCALL_P(SYS_writev, __sys_writev),
	SYSCALL_P(SYS_exit, __sys_exit),
	SYSCALL_P(SYS_brk, __sys_brk),
	SYSCALL_P(SYS_mmap, __sys_mmap),
	SYSCALL_P(SYS_munmap, __sys_munmap),
	SYSCALL_P(SYS_madvise, __sys_madvise),
	SYSCALL_P(SYS_mprotect, __sys_mprotect),
	SYSCALL_P(SYS_clone, __sys_clone),

	/* system dependent */
	SYSCALL_P(SYS_context_switch, __sys_context_switch),
};
