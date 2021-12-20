/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#include <bmetal/syscall.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>

intptr_t __sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	printk("unknown syscall %"PRIdPTR"\n", number);

	return -ENOTSUP;
}

static int real_fd(int fd)
{
	struct __process_info *pi = __get_current_process();
	int rfd;

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		printk("real_fd: fd %d is invalid\n", fd);
		return -1;
	}

	rfd = pi->fdset[fd];

	printk("real_fd: fd:%d -> %d\n", fd, rfd);

	return rfd;
}

ssize_t __sys_write(int fd, const void *buf, size_t count)
{
	int rfd = real_fd(fd);
	ssize_t ret = 0;

	if (count == 0) {
		return 0;
	}

	printk("SYS_write(): fd:%d rfd:%d cnt:%d rd:%d\n", fd, rfd, (int)count, (int)ret);

	return ret;
}
