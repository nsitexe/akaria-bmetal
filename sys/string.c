/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/string.h>

void *kmemset(void *s, int c, size_t n)
{
	char *p = s;

	for (; n; n--, p++) {
		*p = c;
	}

	return s;
}

void *kmemcpy(void *dest, const void *src, size_t n)
{
	char *pd = dest;
	const char *ps = src;

	for (; n; n--, pd++, ps++) {
		*pd = *ps;
	}

	return dest;
}

size_t kstrlen(const char *s)
{
	const char *p = s;

	while (*p != '\0') {
		p++;
	}

	return p - s;
}

int kstrcmp(const char *s1, const char *s2)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	while (*p1 != '\0' && *p1 == *p2) {
		p1++;
		p2++;
	}

	return *p1 - *p2;
}

int kstrncmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	while (n > 0 && *p1 == *p2) {
		if (n == 0 || *p1 == '\0') {
			break;
		}
		p1++;
		p2++;
		n--;
	}

	return *p1 - *p2;
}

static int kisupper(char c)
{
	/* Do not support locale */
	return 'A' <= c && c <= 'Z';
}

static int ktolower(char c)
{
	/* Do not support multibytes */
	return kisupper(c) ? (c - 'A' + 'a') : c;
}

int kstrcasecmp(const char *s1, const char *s2)
{
	int c1, c2, d = 0;

	while (1) {
		c1 = ktolower(*s1);
		c2 = ktolower(*s2);
		d = c1 - c2;
		if (c1 == '\0' || d != 0) {
			break;
		}

		s1++;
		s2++;
	}

	return d;
}

int kstrncasecmp(const char *s1, const char *s2, size_t n)
{
	int c1, c2, d = 0;

	while (n > 0) {
		c1 = ktolower(*s1);
		c2 = ktolower(*s2);
		d = c1 - c2;
		if (c1 == '\0' || d != 0) {
			break;
		}

		s1++;
		s2++;
		n--;
	}

	return d;
}
