/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYSCALL_H_
#define BAREMETAL_CRT_SYSCALL_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/syscall_num.h>
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

long __sys_uname(struct new_utsname *name);
long __sys_getuid(void);
long __sys_geteuid(void);
long __sys_getgid(void);
long __sys_getegid(void);
long __sys_getpid(void);
long __sys_gettid(void);
long __sys_close(int fd);
ssize_t __sys_write(int fd, const void *buf, size_t count);
ssize_t __sys_writev(int fd, const struct iovec *iov, int iovcnt);
long __sys_exit_group(int status);
long __sys_exit(int status);
void *__sys_brk(void *addr);
void *__sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
long __sys_munmap(void *addr, size_t length);
long __sys_madvise(void *addr, size_t length, int advice);
long __sys_mprotect(void *addr, size_t length, int prot);
#if defined(CONFIG_RISCV)
long __sys_clone(unsigned long flags, void *child_stack, void *ptid, void *tls, void *ctid);
#endif
long __sys_futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3);
long __sys_context_switch(void);

#endif /* BAREMETAL_CRT_SYSCALL_H_ */
