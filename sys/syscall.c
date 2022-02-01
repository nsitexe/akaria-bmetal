/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#include <bmetal/syscall.h>
#include <bmetal/comm.h>
#include <bmetal/file.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>

intptr_t __sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	printk("unknown syscall %"PRIdPTR"\n", number);

	return -ENOTSUP;
}

static struct __file_desc *get_file_desc(int fd)
{
	struct __process_info *pi = __get_current_process();

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		printk("get_file_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	return pi->fdset[fd];
}

ssize_t __sys_write(int fd, const void *buf, size_t count)
{
	struct __file_desc *desc = get_file_desc(fd);
	ssize_t ret = 0;

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}
	if (count == 0) {
		return 0;
	}

	//printk("SYS_write(): fd:%d cnt:%d rd:%d\n", fd, (int)count, (int)ret);

	if (desc->ops && desc->ops->write) {
		ret = desc->ops->write(desc, buf, count);
	}

	return ret;
}

void __sys_exit(int status)
{
	struct __comm_area_header *h_area = (struct __comm_area_header *)__comm_area;

	h_area->ret_main = status;
	h_area->done = 1;
}
