/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_STRING_H_
#define BAREMETAL_CRT_SYS_STRING_H_

#include <stddef.h>

#include <bmetal/bmetal.h>

void *kmemset(void *s, int c, size_t n);
void *kmemcpy(void *dest, const void *src, size_t n);
size_t kstrlen(const char *s);
int kstrcmp(const char *s1, const char *s2);
int kstrncmp(const char *s1, const char *s2, size_t n);
int kstrcasecmp(const char *s1, const char *s2);
int kstrncasecmp(const char *s1, const char *s2, size_t n);

#endif /* BAREMETAL_CRT_SYS_STRING_H_ */
