/* SPDX-License-Identifier: Apache-2.0 */

#include <stdarg.h>
#include <stddef.h>

#include <bmetal/printk.h>
#include <bmetal/intr.h>
#include <bmetal/lock.h>
#include <bmetal/sys/stdio.h>
#include <bmetal/sys/string.h>

int inner_vprintf(const char *format, va_list va);
int inner_vsnprintf(char *buffer, size_t count, const char *format, va_list va);

static int null_getc(void);
static int null_putc(int c);

static __getc_func printk_getc = null_getc;
static __putc_func printk_putc = null_putc;
static struct __spinlock printk_lock;

static int null_getc(void)
{
	return EOF;
}

static int null_putc(int c)
{
	return (unsigned char)c;
}

int inner_getc(void)
{
	return printk_getc();
}

int inner_putc(int c)
{
	return printk_putc(c);
}

static int inner_puts(const char *s, int newline)
{
	for (size_t i = 0; i < kstrlen(s); i++) {
		inner_putc(s[i]);
	}
	if (newline) {
		inner_putc('\n');
	}

	return 0;
}

__getc_func __get_printk_in(void)
{
	return printk_getc;
}

int __set_printk_in(__getc_func f)
{
	if (f == NULL) {
		f = null_getc;
	}

	printk_getc = f;

	return 0;
}

__putc_func __get_printk_out(void)
{
	return printk_putc;
}

int __set_printk_out(__putc_func f)
{
	if (f == NULL) {
		f = null_putc;
	}

	printk_putc = f;

	return 0;
}

int kputchar(int c)
{
	long st;
	int r;

	__intr_save_local(&st);
	__spinlock_lock(&printk_lock);
	r = inner_putc(c);
	__spinlock_unlock(&printk_lock);
	__intr_restore_local(st);

	return r;
}

int kputs(const char *s)
{
	long st;
	int r;

	__intr_save_local(&st);
	__spinlock_lock(&printk_lock);
	r = inner_puts(s, 1);
	__spinlock_unlock(&printk_lock);
	__intr_restore_local(st);

	return r;
}

int __kread(char *s, size_t count)
{
	long st;
	int c, n;

	__intr_save_local(&st);
	__spinlock_lock(&printk_lock);
	c = inner_getc();
	if (c == EOF) {
		n = 0;
	} else {
		s[0] = c;
		n = 1;
	}
	__spinlock_unlock(&printk_lock);
	__intr_restore_local(st);

	return n;
}

int __kwrite(const char *s, size_t count)
{
	long st;

	__intr_save_local(&st);
	__spinlock_lock(&printk_lock);
	for (size_t i = 0; i < count; i++) {
		inner_putc(s[i]);
	}
	__spinlock_unlock(&printk_lock);
	__intr_restore_local(st);

	return count;
}

int printk(const char *format, ...)
{
	va_list va;
	int r;

	va_start(va, format);
	r = vprintk(format, va);
	va_end(va);

	return r;
}

int sprintk(char *buffer, const char *format, ...)
{
	va_list va;
	int r;

	va_start(va, format);
	r = vsprintk(buffer, format, va);
	va_end(va);

	return r;
}

int snprintk(char *buffer, size_t count, const char *format, ...)
{
	va_list va;
	int r;

	va_start(va, format);
	r = vsnprintk(buffer, count, format, va);
	va_end(va);

	return r;
}

int vprintk(const char *format, va_list va)
{
	long st;
	int r;

	__intr_save_local(&st);
	__spinlock_lock(&printk_lock);
	r = inner_vprintf(format, va);
	__spinlock_unlock(&printk_lock);
	__intr_restore_local(st);

	return r;
}

int vsprintk(char *buffer, const char *format, va_list va)
{
	return vsnprintk(buffer, (size_t) -1, format, va);
}

int vsnprintk(char *buffer, size_t count, const char *format, va_list va)
{
	long st;
	int r;

	__intr_save_local(&st);
	__spinlock_lock(&printk_lock);
	r = inner_vsnprintf(buffer, count, format, va);
	__spinlock_unlock(&printk_lock);
	__intr_restore_local(st);

	return r;
}
