/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_PRINTK_H_
#define BAREMETAL_CRT_PRINTK_H_

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

#define __pri_level(lv, fmt, ...)                   \
	do {                                        \
		if (PRI_LV_CUR <= lv) {             \
			printk(fmt, ##__VA_ARGS__); \
		}                                   \
	} while (0)
#define pri_err(fmt, ...)     __pri_level(PRI_LV_ERR, fmt, ##__VA_ARGS__)
#define pri_warn(fmt, ...)    __pri_level(PRI_LV_WARN, fmt, ##__VA_ARGS__)
#define pri_info(fmt, ...)    __pri_level(PRI_LV_INFO, fmt, ##__VA_ARGS__)
#define pri_dbg(fmt, ...)     __pri_level(PRI_LV_DBG, fmt, ##__VA_ARGS__)

typedef int (*__putc_func)(int c);

int kputchar(int c);
int kputs(const char *s);
int __kwrite(const char *s, size_t count);
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
