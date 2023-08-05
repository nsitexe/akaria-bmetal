/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/init.h>
#include <bmetal/file.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/sys/string.h>

static const struct __file_ops file_stdio_ops = {
	.read = __file_stdio_read,
	.write = __file_stdio_write,
};

static struct __file_desc fds[CONFIG_MAX_FD] = {
	/* stdin */
	[0] = {
		.ops = &file_stdio_ops,
	},

	/* stdout */
	[1] = {
		.ops = &file_stdio_ops,
	},

	/* stderr */
	[2] = {
		.ops = &file_stdio_ops,
	},
};

struct __file_desc *__file_get_desc(int fd)
{
	struct __proc_info *pi = __proc_get_current();

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		pri_info("__file_get_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	return pi->fdset[fd];
}

struct __file_desc *__file_set_desc(int fd, struct __file_desc *desc)
{
	struct __proc_info *pi = __proc_get_current();
	struct __file_desc *olddesc;

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		pri_info("__file_set_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	olddesc = pi->fdset[fd];
	pi->fdset[fd] = desc;

	return olddesc;
}

ssize_t __file_read_nolock(struct __file_desc *desc, void *buf, size_t count)
{
	ssize_t ret = 0;

	if (!buf) {
		return -EINVAL;
	}
	if (count == 0) {
		return -EINVAL;
	}

	if (desc->ops && desc->ops->read) {
		ret = desc->ops->read(desc, buf, count);
	}

	return ret;
}

ssize_t __file_read(struct __file_desc *desc, void *buf, size_t count)
{
	ssize_t ret;

	__spinlock_lock(&desc->lock);
	ret = __file_read_nolock(desc, buf, count);
	__spinlock_unlock(&desc->lock);

	return ret;
}

ssize_t __file_write_nolock(struct __file_desc *desc, const void *buf, size_t count)
{
	ssize_t ret = 0;

	if (!buf) {
		return -EINVAL;
	}
	if (count == 0) {
		return 0;
	}

	if (desc->ops && desc->ops->write) {
		ret = desc->ops->write(desc, buf, count);
	}

	return ret;
}

ssize_t __file_write(struct __file_desc *desc, const void *buf, size_t count)
{
	ssize_t ret;

	__spinlock_lock(&desc->lock);
	ret = __file_write_nolock(desc, buf, count);
	__spinlock_unlock(&desc->lock);

	return ret;
}

ssize_t __file_stdio_read(struct __file_desc *desc, void *buf, size_t count)
{
	return __kread(buf, count);
}

ssize_t __file_stdio_write(struct __file_desc *desc, const void *buf, size_t count)
{
	return __kwrite(buf, count);
}

int __file_stdio_init(struct __proc_info *pi)
{
	__file_set_desc(0, &fds[0]);
	__file_set_desc(1, &fds[1]);
	__file_set_desc(2, &fds[2]);

	return 0;
}
