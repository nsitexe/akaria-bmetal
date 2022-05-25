/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_UIO_H_
#define BAREMETAL_CRT_SYS_UIO_H_

#include <stddef.h>

#include <bmetal/bmetal.h>

struct iovec {
	void *iov_base;
	size_t iov_len;
};

#endif /* BAREMETAL_CRT_SYS_UIO_H_ */
