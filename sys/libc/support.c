/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>
#include <bmetal/syscall.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/string.h>

void _start(void);

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
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_INTADDR, (uintptr_t)_start);

	return 0;
}

const __syscall_func_t __table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, __sys_wrap_uname),
	SYSCALL_P(SYS_prlimit64, __sys_wrap_prlimit64),
	SYSCALL_P(SYS_getuid, __sys_wrap_getuid),
	SYSCALL_P(SYS_geteuid, __sys_wrap_geteuid),
	SYSCALL_P(SYS_getgid, __sys_wrap_getgid),
	SYSCALL_P(SYS_getegid, __sys_wrap_getegid),
	SYSCALL_P(SYS_getpid, __sys_wrap_getpid),
	SYSCALL_P(SYS_gettid, __sys_wrap_gettid),
#ifdef CONFIG_64BIT
	SYSCALL_P(SYS_clock_gettime, __sys_wrap_clock_gettime64),
	SYSCALL_P(SYS_clock_settime, __sys_wrap_clock_settime64),
#else /* CONFIG_64BIT */
	SYSCALL_P(SYS_clock_gettime, __sys_wrap_clock_gettime32),
	SYSCALL_P(SYS_clock_settime, __sys_wrap_clock_settime32),
	SYSCALL_P(SYS_clock_gettime64, __sys_wrap_clock_gettime64),
	SYSCALL_P(SYS_clock_settime64, __sys_wrap_clock_settime64),
#endif /* CONFIG_64BIT */
	SYSCALL_P(SYS_gettimeofday, __sys_wrap_gettimeofday),
	SYSCALL_P(SYS_settimeofday, __sys_wrap_settimeofday),
	SYSCALL_P(SYS_getrandom, __sys_wrap_getrandom),
	SYSCALL_P(SYS_close, __sys_wrap_close),
	SYSCALL_P(SYS_read, __sys_wrap_read),
	SYSCALL_P(SYS_write, __sys_wrap_write),
	SYSCALL_P(SYS_writev, __sys_wrap_writev),
	SYSCALL_P(SYS_brk, __sys_wrap_brk),
	SYSCALL_P(SYS_mmap, __sys_wrap_mmap),
	SYSCALL_P(SYS_munmap, __sys_wrap_munmap),
	SYSCALL_P(SYS_madvise, __sys_wrap_madvise),
	SYSCALL_P(SYS_mprotect, __sys_wrap_mprotect),
	SYSCALL_P(SYS_clone, __sys_wrap_clone),
#ifdef CONFIG_64BIT
	SYSCALL_P(SYS_futex, __sys_wrap_futex64),
#else /* CONFIG_64BIT */
	SYSCALL_P(SYS_futex, __sys_wrap_futex32),
	SYSCALL_P(SYS_futex_time64, __sys_wrap_futex64),
#endif /* CONFIG_64BIT */
	SYSCALL_P(SYS_set_robust_list, __sys_wrap_set_robust_list),
	SYSCALL_P(SYS_set_tid_address, __sys_wrap_set_tid_address),
	SYSCALL_P(SYS_exit_group, __sys_wrap_exit_group),
	SYSCALL_P(SYS_exit, __sys_wrap_exit),
	SYSCALL_P(SYS_reboot, __sys_wrap_reboot),

	/* system dependent */
	SYSCALL_P(SYS_context_switch, __sys_context_switch),
};
