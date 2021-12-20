/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_APP_SYSCALL_H_
#define BAREMETAL_CRT_APP_SYSCALL_H_

#include <bmetal/bmetal.h>

#include <stdint.h>
#include <unistd.h>

typedef intptr_t (*__syscall_func_t)(intptr_t no, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

intptr_t __sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

ssize_t __sys_write(int fd, const void *buf, size_t count);

#endif /* BAREMETAL_CRT_APP_SYSCALL_H_ */
