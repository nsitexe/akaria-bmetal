/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_FILE_H_
#define BAREMETAL_CRT_FILE_H_

#include <sys/types.h>

#include <bmetal/bmetal.h>

struct __file_desc;
struct __process_info;

struct __file_ops {
	ssize_t (*write)(struct __file_desc *desc, const void *buf, size_t count);
};

struct __file_desc {
	const struct __file_ops *ops;
};

ssize_t __file_stdio_write(struct __file_desc *desc, const void *buf, size_t count);
int __file_stdio_init(struct __process_info *pi);

#endif /* BAREMETAL_CRT_FILE_H_ */
