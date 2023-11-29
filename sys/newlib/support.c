/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>
#include <bmetal/syscall.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/string.h>

void k_crt_start(void);

int k_libc_init_main_thread(struct k_thread_info *ti, int argc, char *argv[], char *envp[], char *sp_user, char *sp_intr)
{
	intptr_t *v;

	sp_user = (void *)argv;
	sp_user -= sizeof(intptr_t);
	v = (intptr_t *)sp_user;
	*v = argc;

	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_1, 0);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_STACK, (uintptr_t)sp_user);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_STACK_INTR, (uintptr_t)sp_intr);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_INTADDR, (uintptr_t)k_crt_start);

	return 0;
}

const k_syscall_func_t k_table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, k_sys_wrap_uname),
	SYSCALL_P(SYS_getuid, k_sys_wrap_getuid),
	SYSCALL_P(SYS_geteuid, k_sys_wrap_geteuid),
	SYSCALL_P(SYS_getgid, k_sys_wrap_getgid),
	SYSCALL_P(SYS_getegid, k_sys_wrap_getegid),
	SYSCALL_P(SYS_getpid, k_sys_wrap_getpid),
#ifdef CONFIG_64BIT
#else /* CONFIG_64BIT */
	SYSCALL_P(SYS_clock_gettime64, k_sys_wrap_clock_gettime64),
#endif /* CONFIG_64BIT */
	SYSCALL_P(SYS_gettimeofday, k_sys_wrap_gettimeofday),
	SYSCALL_P(SYS_openat, k_sys_wrap_openat),
	SYSCALL_P(SYS_close, k_sys_wrap_close),
	SYSCALL_P(SYS_read, k_sys_wrap_read),
	SYSCALL_P(SYS_write, k_sys_wrap_write),
	SYSCALL_P(SYS_writev, k_sys_wrap_writev),
	SYSCALL_P(SYS_brk, k_sys_wrap_brk),
	SYSCALL_P(SYS_mmap, k_sys_wrap_mmap),
	SYSCALL_P(SYS_munmap, k_sys_wrap_munmap),
	SYSCALL_P(SYS_exit_group, k_sys_wrap_exit_group),
	SYSCALL_P(SYS_exit, k_sys_wrap_exit),

	/* system dependent */
	SYSCALL_P(SYS_context_switch, k_sys_wrap_context_switch),
};
