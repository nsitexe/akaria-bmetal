/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYSCALL_H_
#define BAREMETAL_CRT_SYSCALL_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/syscall_num.h>
#include <bmetal/sys/resource.h>
#include <bmetal/sys/time.h>
#include <bmetal/sys/uio.h>

#define SYSCALL_P(num, func)    [(num)] = (__syscall_func_t)(func)

#define __NEW_UTS_LEN 64
#define __PAGE_SIZE   4096

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

intptr_t __sys_uname(struct new_utsname *name);
intptr_t __sys_prlimit64(pid_t pid, int resource, const struct rlimit64 *new_lim, struct rlimit64 *old_lim);
intptr_t __sys_getuid(void);
intptr_t __sys_geteuid(void);
intptr_t __sys_getgid(void);
intptr_t __sys_getegid(void);
intptr_t __sys_getpid(void);
intptr_t __sys_gettid(void);
intptr_t __sys_clock_gettime(clockid_t clock_id, struct timespec64 *tp);
intptr_t __sys_clock_settime(clockid_t clock_id, const struct timespec64 *tp);
intptr_t __sys_gettimeofday(struct timeval *tp, void *tzp);
intptr_t __sys_settimeofday(const struct timeval *tp, const void *tzp);
intptr_t __sys_getrandom(void *buf, size_t buflen, unsigned int flags);
intptr_t __sys_close(int fd);
intptr_t __sys_read(int fd, void *buf, size_t count);
intptr_t __sys_write(int fd, const void *buf, size_t count);
intptr_t __sys_writev(int fd, const struct iovec *iov, int iovcnt);
void *__brk_area_start(void);
void *__brk_area_end(void);
intptr_t __sys_brk(void *addr);
void *__heap_area_start(void);
void *__heap_area_end(void);
intptr_t __sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
intptr_t __sys_munmap(void *addr, size_t length);
intptr_t __sys_madvise(void *addr, size_t length, int advice);
intptr_t __sys_mprotect(void *addr, size_t length, int prot);
#if defined(CONFIG_RISCV)
intptr_t __sys_clone(unsigned long flags, void *child_stack, void *ptid, void *tls, void *ctid);
#endif
intptr_t __sys_futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3);
intptr_t __sys_set_robust_list(void *head, size_t len);
intptr_t __sys_set_tid_address(int *tidptr);
intptr_t __sys_exit_group(int status);
intptr_t __sys_exit(int status);
intptr_t __sys_reboot(int magic, int magic2, int cmd);
intptr_t __sys_context_switch(void);

#endif /* BAREMETAL_CRT_SYSCALL_H_ */
