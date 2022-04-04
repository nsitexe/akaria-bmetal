/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_PRINTK_H_
#define BAREMETAL_CRT_PRINTK_H_

#include <stdarg.h>

#include <bmetal/bmetal.h>

typedef int (*__putc_func)(int c);

int kputchar(int c);
int kputs(const char *s);
int __attribute__((format(printf, 1, 2))) printk(const char *format, ...);
int __attribute__((format(printf, 2, 3))) sprintk(char *buffer, const char *format, ...);
int __attribute__((format(printf, 3, 4))) snprintk(char *buffer, size_t count, const char *format, ...);
int vprintk(const char *format, va_list va);
int vsprintk(char *buffer, const char *format, va_list va);
int vsnprintk(char *buffer, size_t count, const char *format, va_list va);
int __attribute__((format(printf, 3, 4))) fctprintk(void (*out)(char character, void *arg), void *arg, const char *format, ...);

__putc_func __get_printk_out(void);
int __set_printk_out(__putc_func f);

#endif /* BAREMETAL_CRT_PRINTK_H_ */
