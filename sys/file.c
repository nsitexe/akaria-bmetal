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
	pi->fdset[0] = &fds[0];
	pi->fdset[1] = &fds[1];
	pi->fdset[2] = &fds[2];

	return 0;
}
