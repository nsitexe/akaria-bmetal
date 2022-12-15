/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_FILE_H_
#define BAREMETAL_CRT_FILE_H_

#include <stddef.h>

#include <bmetal/bmetal.h>
#include <bmetal/lock.h>
#include <bmetal/sys/types.h>

#if !defined(__ASSEMBLER__)

struct __file_desc;
struct __proc_info;

struct __file_ops {
	ssize_t (*read)(struct __file_desc *desc, void *buf, size_t count);
	ssize_t (*write)(struct __file_desc *desc, const void *buf, size_t count);
	int (*close)(struct __file_desc *desc);
};

struct __file_desc {
	struct __spinlock lock;
	const struct __file_ops *ops;
};

ssize_t __file_stdio_read(struct __file_desc *desc, void *buf, size_t count);
ssize_t __file_stdio_write(struct __file_desc *desc, const void *buf, size_t count);
int __file_stdio_init(struct __proc_info *pi);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_FILE_H_ */
