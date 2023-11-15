/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_STRING_H_
#define BAREMETAL_CRT_SYS_STRING_H_

#include <stddef.h>

#include <bmetal/bmetal.h>

void *__kmemset(void *s, int c, size_t n);
void *__kmemcpy(void *dest, const void *src, size_t n);
size_t __kstrlen(const char *s);
int __kstrcmp(const char *s1, const char *s2);
int __kstrncmp(const char *s1, const char *s2, size_t n);
int __kstrcasecmp(const char *s1, const char *s2);
int __kstrncasecmp(const char *s1, const char *s2, size_t n);

#endif /* BAREMETAL_CRT_SYS_STRING_H_ */
