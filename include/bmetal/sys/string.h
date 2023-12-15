/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_STRING_H_
#define BAREMETAL_CRT_SYS_STRING_H_

#include <stddef.h>

#include <bmetal/bmetal.h>

void *k_memset(void *s, int c, size_t n);
void *k_memcpy(void *dest, const void *src, size_t n);
size_t k_strlen(const char *s);
int k_strcmp(const char *s1, const char *s2);
int k_strncmp(const char *s1, const char *s2, size_t n);
int k_strcasecmp(const char *s1, const char *s2);
int k_strncasecmp(const char *s1, const char *s2, size_t n);

#endif /* BAREMETAL_CRT_SYS_STRING_H_ */
