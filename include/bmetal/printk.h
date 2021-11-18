/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_PRINTK_H_
#define BAREMETAL_CRT_PRINTK_H_

#include <stdarg.h>

#include <bmetal/bmetal.h>

typedef int (*__putc_func)(int c);

int __attribute__((format(printf, 1, 2))) printk(const char *fmt, ...);
int vprintk(const char *fmt, va_list ap);

__putc_func get_printk_out(void);
int set_printk_out(__putc_func f);

#endif /* BAREMETAL_CRT_PRINTK_H_ */
