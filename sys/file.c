/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/init.h>
#include <bmetal/file.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/sys/string.h>

static const struct k_file_ops file_stdio_ops = {
	.read = k_file_stdio_read,
	.write = k_file_stdio_write,
};

static struct k_file_desc fds[CONFIG_MAX_FD] = {
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

struct k_file_desc *k_file_get_desc(int fd)
{
	struct k_proc_info *pi = k_proc_get_current();

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		pri_info("k_file_get_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	return pi->fdset[fd];
}

struct k_file_desc *k_file_set_desc(int fd, struct k_file_desc *desc)
{
	struct k_proc_info *pi = k_proc_get_current();
	struct k_file_desc *olddesc;

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		pri_info("k_file_set_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	olddesc = pi->fdset[fd];
	pi->fdset[fd] = desc;

	return olddesc;
}

ssize_t k_file_read_nolock(struct k_file_desc *desc, void *buf, size_t count)
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

ssize_t k_file_read(struct k_file_desc *desc, void *buf, size_t count)
{
	ssize_t ret;

	__spinlock_lock(&desc->lock);
	ret = k_file_read_nolock(desc, buf, count);
	__spinlock_unlock(&desc->lock);

	return ret;
}

ssize_t k_file_write_nolock(struct k_file_desc *desc, const void *buf, size_t count)
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

ssize_t k_file_write(struct k_file_desc *desc, const void *buf, size_t count)
{
	ssize_t ret;

	__spinlock_lock(&desc->lock);
	ret = k_file_write_nolock(desc, buf, count);
	__spinlock_unlock(&desc->lock);

	return ret;
}

ssize_t k_file_stdio_read(struct k_file_desc *desc, void *buf, size_t count)
{
	return __kread(buf, count);
}

ssize_t k_file_stdio_write(struct k_file_desc *desc, const void *buf, size_t count)
{
	return __kwrite(buf, count);
}

int k_file_stdio_init(struct k_proc_info *pi)
{
	k_file_set_desc(0, &fds[0]);
	k_file_set_desc(1, &fds[1]);
	k_file_set_desc(2, &fds[2]);

	return 0;
}
