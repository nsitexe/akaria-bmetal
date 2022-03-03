/* SPDX-License-Identifier: Apache-2.0 */

#include <stdarg.h>
#include <stdio.h>

#include <bmetal/printk.h>
#include <bmetal/lock.h>
#include <bmetal/string.h>

static int null_putc(int c);

static __putc_func printk_putc = null_putc;
static struct __spinlock printk_lock;

static int null_putc(int c)
{
	return (unsigned char)c;
}

static int inner_putc(int c)
{
	return printk_putc(c);
}

static int inner_puts(const char *s, int newline)
{
	__spinlock_lock(&printk_lock);

	for (size_t i = 0; i < kstrlen(s); i++) {
		inner_putc(s[i]);
	}
	if (newline) {
		inner_putc('\n');
	}

	__spinlock_unlock(&printk_lock);

	return 0;
}

__putc_func __get_printk_out(void)
{
	return printk_putc;
}

int __set_printk_out(__putc_func f)
{
	if (f == NULL)
		f = null_putc;

	printk_putc = f;

	return 0;
}

int __attribute__((format(printf, 1, 2))) printk(const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vprintk(fmt, ap);
	va_end(ap);

	return r;
}

int vprintk(const char *fmt, va_list ap)
{
	char tmp[256];
	int r;

	r = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	inner_puts(tmp, 0);

	return r;
}
