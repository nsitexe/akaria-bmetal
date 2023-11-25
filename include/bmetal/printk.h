/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_PRINT_H_
#define BAREMETAL_CRT_PRINT_H_

#include <stdarg.h>
#include <stddef.h>

#include <bmetal/bmetal.h>

#define PRI_LV_ERR     50
#define PRI_LV_WARN    40
#define PRI_LV_INFO    30
#define PRI_LV_DBG     20

#define PRI_LV_CUR     PRI_LV_WARN

#ifdef CONFIG_PRINT_INFO
#undef  PRI_LV_CUR
#define PRI_LV_CUR     PRI_LV_INFO
#endif

#ifdef CONFIG_PRINT_DEBUG
#undef  PRI_LV_CUR
#define PRI_LV_CUR     PRI_LV_DBG
#endif

#if !defined(__ASSEMBLER__)

#define k_pri_level(lv, fmt, ...)                     \
	do {                                          \
		if (PRI_LV_CUR <= lv) {               \
			k_printf(fmt, ##__VA_ARGS__); \
		}                                     \
	} while (0)
#define pri_err(fmt, ...)     k_pri_level(PRI_LV_ERR, fmt, ##__VA_ARGS__)
#define pri_warn(fmt, ...)    k_pri_level(PRI_LV_WARN, fmt, ##__VA_ARGS__)
#define pri_info(fmt, ...)    k_pri_level(PRI_LV_INFO, fmt, ##__VA_ARGS__)
#define pri_dbg(fmt, ...)     k_pri_level(PRI_LV_DBG, fmt, ##__VA_ARGS__)

typedef int (*k_pri_getchar_func)(void);
typedef int (*k_pri_putchar_func)(int c);

k_pri_getchar_func k_pri_get_getchar(void);
int k_pri_set_getchar(k_pri_getchar_func f);
k_pri_putchar_func k_pri_get_putchar(void);
int k_pri_set_putchar(k_pri_putchar_func f);

int k_getchar_nolock(void);
int k_putchar_nolock(int c);
int k_puts_nolock(const char *s, int newline);
int k_pri_read_stdin(char *s, size_t count);
int k_pri_write_stdout(const char *s, size_t count);

int k_putchar(int c);
int k_puts(const char *s);
int __attribute__((format(printf, 1, 2))) k_printf(const char *format, ...);
int __attribute__((format(printf, 2, 3))) k_sprintf(char *buffer, const char *format, ...);
int __attribute__((format(printf, 3, 4))) k_snprintf(char *buffer, size_t count, const char *format, ...);
int k_vprintf(const char *format, va_list va);
int k_vsprintf(char *buffer, const char *format, va_list va);
int k_vsnprintf(char *buffer, size_t count, const char *format, va_list va);

int k_vprintf_nolock(const char* format, va_list va);
int k_vsnprintf_nolock(char* buffer, size_t count, const char* format, va_list va);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_PRINT_H_ */
