/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_UIO_H_
#define BAREMETAL_CRT_SYS_UIO_H_

#if !defined(__ASSEMBLER__)
#include <stddef.h>
#endif /* !__ASSEMBLER__ */

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

struct iovec {
	void *iov_base;
	size_t iov_len;
};

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SYS_UIO_H_ */
