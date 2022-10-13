/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>
#include <bmetal/sys/string.h>

#define is_aligned(p, n)    (((uintptr_t)(p) % (n)) == 0)

#ifdef CONFIG_64BIT
#define OPT_T        uint64_t
#define OPT_BYTES    8
#else /* CONFIG_64BIT */
#define OPT_T        uint32_t
#define OPT_BYTES    4
#endif /* CONFIG_64BIT */

void *kmemset(void *s, int c, size_t n)
{
	void *s_org = s;

	for (; !is_aligned(s, OPT_BYTES) && n; n--, s++) {
		*(char *)s = c;
	}

	if (is_aligned(s, OPT_BYTES)) {
		OPT_T c2 = (uint8_t)c;

		c2 |= c2 << 8;
		c2 |= c2 << 16;
		if (OPT_BYTES > 4) {
			/* Avoid warning even if OPT_T is 32bits */
			c2 |= (c2 << 16) << 16;
		}
		for (; n > OPT_BYTES * 4; n -= OPT_BYTES * 4, s += OPT_BYTES * 4) {
			OPT_T *ps = s;

			ps[0] = c2;
			ps[1] = c2;
			ps[2] = c2;
			ps[3] = c2;
		}
		for (; n > OPT_BYTES; n -= OPT_BYTES, s += OPT_BYTES) {
			*(OPT_T *)s = c2;
		}
	}

	for (; n; n--, s++) {
		*(char *)s = c;
	}

	return s_org;
}

void *kmemcpy(void *dest, const void *src, size_t n)
{
	void *dest_org = dest;

	if (is_aligned(dest, OPT_BYTES) && is_aligned(src, OPT_BYTES)) {
		const size_t step = OPT_BYTES * 4;

		for (; n > step; n -= step, dest += step, src += step) {
			const OPT_T *psrc = src;
			OPT_T *pdest = dest;

			pdest[0] = psrc[0];
			pdest[1] = psrc[1];
			pdest[2] = psrc[2];
			pdest[3] = psrc[3];
		}
		for (; n > OPT_BYTES; n -= OPT_BYTES, dest += OPT_BYTES, src += OPT_BYTES) {
			*(OPT_T *)dest = *(const OPT_T *)src;
		}
	}

	for (; n; n--, dest++, src++) {
		*(char *)dest = *(const char *)src;
	}

	return dest_org;
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

	if (s1 == s2) {
		return 0;
	}

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

	if (s1 == s2) {
		return 0;
	}

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

	if (s1 == s2) {
		return 0;
	}

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

	if (s1 == s2) {
		return 0;
	}

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
