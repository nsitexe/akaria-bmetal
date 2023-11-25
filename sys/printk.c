/* SPDX-License-Identifier: Apache-2.0 */

#include <stdarg.h>
#include <stddef.h>

#include <bmetal/printk.h>
#include <bmetal/intr.h>
#include <bmetal/lock.h>
#include <bmetal/sys/stdio.h>
#include <bmetal/sys/string.h>

static int null_getchar(void);
static int null_putchar(int c);

static k_pri_getchar_func k_pri_getchar_in = null_getchar;
static k_pri_putchar_func k_pri_putchar_in = null_putchar;
static struct k_spinlock k_pri_lock;

static int null_getchar(void)
{
	return EOF;
}

static int null_putchar(int c)
{
	return (unsigned char)c;
}

k_pri_getchar_func k_pri_get_getchar(void)
{
	return k_pri_getchar_in;
}

int k_pri_set_getchar(k_pri_getchar_func f)
{
	if (f == NULL) {
		f = null_getchar;
	}

	k_pri_getchar_in = f;

	return 0;
}

k_pri_putchar_func k_pri_get_putchar(void)
{
	return k_pri_putchar_in;
}

int k_pri_set_putchar(k_pri_putchar_func f)
{
	if (f == NULL) {
		f = null_putchar;
	}

	k_pri_putchar_in = f;

	return 0;
}

int k_getchar_nolock(void)
{
	return k_pri_getchar_in();
}

int k_putchar_nolock(int c)
{
	return k_pri_putchar_in(c);
}

int k_puts_nolock(const char *s, int newline)
{
	for (size_t i = 0; i < k_strlen(s); i++) {
		k_putchar_nolock(s[i]);
	}
	if (newline) {
		k_putchar_nolock('\n');
	}

	return 0;
}

int k_pri_read_stdin(char *s, size_t count)
{
	long st;
	int c, n;

	k_intr_save_local(&st);
	k_spinlock_lock(&k_pri_lock);
	c = k_getchar_nolock();
	if (c == EOF) {
		n = 0;
	} else {
		s[0] = c;
		n = 1;
	}
	k_spinlock_unlock(&k_pri_lock);
	k_intr_restore_local(st);

	return n;
}

int k_pri_write_stdout(const char *s, size_t count)
{
	long st;

	k_intr_save_local(&st);
	k_spinlock_lock(&k_pri_lock);
	for (size_t i = 0; i < count; i++) {
		k_putchar_nolock(s[i]);
	}
	k_spinlock_unlock(&k_pri_lock);
	k_intr_restore_local(st);

	return count;
}

int k_putchar(int c)
{
	long st;
	int r;

	k_intr_save_local(&st);
	k_spinlock_lock(&k_pri_lock);
	r = k_putchar_nolock(c);
	k_spinlock_unlock(&k_pri_lock);
	k_intr_restore_local(st);

	return r;
}

int k_puts(const char *s)
{
	long st;
	int r;

	k_intr_save_local(&st);
	k_spinlock_lock(&k_pri_lock);
	r = k_puts_nolock(s, 1);
	k_spinlock_unlock(&k_pri_lock);
	k_intr_restore_local(st);

	return r;
}

int k_printf(const char *format, ...)
{
	va_list va;
	int r;

	va_start(va, format);
	r = k_vprintf(format, va);
	va_end(va);

	return r;
}

int k_sprintf(char *buffer, const char *format, ...)
{
	va_list va;
	int r;

	va_start(va, format);
	r = k_vsprintf(buffer, format, va);
	va_end(va);

	return r;
}

int k_snprintf(char *buffer, size_t count, const char *format, ...)
{
	va_list va;
	int r;

	va_start(va, format);
	r = k_vsnprintf(buffer, count, format, va);
	va_end(va);

	return r;
}

int k_vprintf(const char *format, va_list va)
{
	long st;
	int r;

	k_intr_save_local(&st);
	k_spinlock_lock(&k_pri_lock);
	r = k_vprintf_nolock(format, va);
	k_spinlock_unlock(&k_pri_lock);
	k_intr_restore_local(st);

	return r;
}

int k_vsprintf(char *buffer, const char *format, va_list va)
{
	return k_vsnprintf(buffer, (size_t) -1, format, va);
}

int k_vsnprintf(char *buffer, size_t count, const char *format, va_list va)
{
	long st;
	int r;

	k_intr_save_local(&st);
	k_spinlock_lock(&k_pri_lock);
	r = k_vsnprintf_nolock(buffer, count, format, va);
	k_spinlock_unlock(&k_pri_lock);
	k_intr_restore_local(st);

	return r;
}
