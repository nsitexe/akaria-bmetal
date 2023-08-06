#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define ARRAY_OF(x)    (sizeof(x) / sizeof((x)[0]))

/* This is not POSIX application */
void *kmemset(void *s, int c, size_t n);
void *kmemcpy(void *dest, const void *src, size_t n);
size_t kstrlen(const char *s);

struct param_memset {
	size_t dest_bufsize;
	size_t dest_start;
	size_t size;
	char c_init;
	int c_set;
	const char *funcname;
	void *(*func)(void *s, int c, size_t n);
};

struct param_memcpy {
	size_t dest_bufsize;
	size_t dest_start;
	size_t src_bufsize;
	size_t src_start;
	size_t size;
	const char *funcname;
	void *(*func)(void *dest, const void *src, size_t n);
};

struct param_memset par_kmemset[] = {
	{0x1000, 1, 1, 0x00, 'a'},
	{0x1000, 1, 1, 0x01, 'b'},
	{0x1000, 0, 1, 0x00, 'c'},
	{0x1000, 0, 2, 0x02, 'd'},
	{0x1000, 0, 3, 0x00, 'e'},
	{0x1000, 0, 4, 0x03, 'f'},
	{0x1000, 0, 0, 0x00, 'g'},
	{0x1000, 1, 0, 0x04, 'h'},
	{0x1000, 2, 0, 0x00, 'i'},
	{0x1000, 3, 0, 0x05, 'j'},
	{0x1000, 4, 0, 0x00, 'k'},

	{0x1000, 0, 4096, 0x06, 'l'},
	{0x1000, 1, 4095, 0x00, 'm'},
	{0x1000, 2, 4094, 0x07, 'n'},
	{0x1000, 3, 4093, 0x00, 'o'},
	{0x1000, 4, 4092, 0x08, 'p'},
	{0x1000, 5, 4091, 0x00, 'q'},
	{0x1000, 6, 4090, 0x09, 'r'},
	{0x1000, 7, 4089, 0x00, 's'},

	{0x1000, 10, 2234, 0x00, 'a'},
	{0x1000, 11, 2234, 0x10, 'b'},
	{0x2000, 12, 2234, 0x11, 'c'},
	{0x3000, 13, 2234, 0x20, 'd'},
	{0x4000, 14, 2234, 0x21, 'e'},
	{0x1000, 15, 2234, 0x30, 'f'},
	{0x2000, 16, 2234, 0x31, 'g'},
	{0x3000, 17, 2234, 0x40, 'h'},
	{0x4000, 18, 2234, 0x41, 'i'},
	{0x4000, 19, 2234, 0x50, 'j'},

	{0x11000, 200, 19234, 0x00, '0'},
	{0x11000, 200, 19235, 0x10, '1'},
	{0x12000, 200, 19236, 0x11, '2'},
	{0x13000, 200, 19237, 0x20, '3'},
	{0x11000, 200, 19238, 0x00, '4'},
	{0x11000, 200, 19239, 0x10, '5'},
	{0x12000, 200, 19240, 0x11, '6'},
	{0x13000, 200, 19241, 0x20, '7'},

	{0x21000, 507, 98234, 0x00, 'A'},
	{0x21000, 507, 98235, 0x10, 'B'},
	{0x22000, 507, 98236, 0x11, 'C'},
	{0x23000, 507, 98237, 0x20, 'D'},
	{0x21000, 507, 98238, 0x00, 'E'},
	{0x21000, 507, 98239, 0x10, 'F'},
	{0x22000, 507, 98240, 0x11, 'G'},
	{0x23000, 507, 98241, 0x20, 'H'},
};

struct param_memcpy par_kmemcpy[] = {
	{0x4000, 0, 0x2000, 1, 0},
	{0x4000, 1, 0x2000, 1, 0},
	{0x4000, 2, 0x2000, 1, 0},
	{0x4000, 3, 0x2000, 1, 0},
	{0x4000, 4, 0x2000, 1, 0},
	{0x4000, 5, 0x2000, 1, 0},
	{0x4000, 6, 0x2000, 1, 0},
	{0x4000, 7, 0x2000, 1, 0},

	{0x4000, 0, 0x2000, 1, 1},
	{0x4000, 1, 0x2000, 1, 1},
	{0x4000, 2, 0x2000, 1, 1},
	{0x4000, 3, 0x2000, 1, 1},
	{0x4000, 4, 0x2000, 1, 1},
	{0x4000, 5, 0x2000, 1, 1},
	{0x4000, 6, 0x2000, 1, 1},
	{0x4000, 7, 0x2000, 1, 1},

	{0x4000, 1, 0x2000, 0, 1},
	{0x4000, 1, 0x2000, 1, 1},
	{0x4000, 1, 0x2000, 2, 1},
	{0x4000, 1, 0x2000, 3, 1},
	{0x4000, 1, 0x2000, 4, 1},
	{0x4000, 1, 0x2000, 5, 1},
	{0x4000, 1, 0x2000, 6, 1},
	{0x4000, 1, 0x2000, 7, 1},

	{0x4000, 210, 0x2000, 310, 3234},
	{0x4000, 211, 0x2000, 310, 3234},
	{0x4000, 212, 0x2000, 310, 3234},
	{0x4000, 213, 0x2000, 310, 3234},
	{0x4000, 214, 0x2000, 310, 3234},
	{0x4000, 215, 0x2000, 310, 3234},
	{0x4000, 216, 0x2000, 310, 3234},
	{0x4000, 217, 0x2000, 310, 3234},

	{0x2000, 110, 0x1000, 410, 2834},
	{0x2000, 110, 0x1000, 411, 2834},
	{0x2000, 110, 0x1000, 412, 2834},
	{0x2000, 110, 0x1000, 413, 2834},
	{0x2000, 110, 0x1000, 414, 2834},
	{0x2000, 110, 0x1000, 415, 2834},
	{0x2000, 110, 0x1000, 416, 2834},
	{0x2000, 110, 0x1000, 417, 2834},

	{0x3000, 310, 0x2000, 510, 3630},
	{0x3000, 310, 0x2000, 510, 3631},
	{0x3000, 310, 0x2000, 510, 3632},
	{0x3000, 310, 0x2000, 510, 3633},
	{0x3000, 310, 0x2000, 510, 3634},
	{0x3000, 310, 0x2000, 510, 3635},
	{0x3000, 310, 0x2000, 510, 3636},
	{0x3000, 310, 0x2000, 510, 3637},
};

int verify_memset_param(const struct param_memset *par)
{
	if (par->dest_bufsize < par->dest_start + par->size) {
		fprintf(stderr, "dest_bufsize:%d is too small "
			"(dest_start:%d + size:%d).\n",
			(int)par->dest_bufsize,
			(int)par->dest_start,
			(int)par->size);
		return -1;
	}

	return 0;
}

int verify_memset(const struct param_memset *par, const char *dest)
{
	/* Check underflow */
	for (size_t i = 0; i < par->dest_start; i++) {
		if (dest[i] != par->c_init) {
			fprintf(stderr, "%s is underflow at %d. "
				"(expect:0x%x != actual:0x%x)\n",
				par->funcname,
				(int)i,
				par->c_init & 0xff,
				dest[i] & 0xff);
			return -1;
		}
	}

	/* Check overflow */
	for (size_t i = par->dest_start + par->size; i < par->dest_bufsize; i++) {
		if (dest[i] != par->c_init) {
			fprintf(stderr, "%s is overflow at %d. "
				"(expect:0x%x != actual:0x%x)\n",
				par->funcname,
				(int)i,
				par->c_init & 0xff,
				dest[i] & 0xff);
			return -1;
		}
	}

	/* Check memset */
	for (size_t i = par->dest_start; i < par->dest_start + par->size; i++) {
		if (dest[i] != par->c_set) {
			fprintf(stderr, "%s is not set at %d. "
				"(expect:0x%x != actual:0x%x)\n",
				par->funcname,
				(int)i,
				par->c_set & 0xff,
				dest[i] & 0xff);
			return -1;
		}
	}

	return 0;
}

int test_memset(const struct param_memset *par)
{
	char *dest, *st;
	void *p;
	int r;

	r = verify_memset_param(par);
	if (r) {
		return r;
	}

	dest = malloc(par->dest_bufsize);
	if (dest == NULL) {
		perror("malloc");
		return -1;
	}

	for (size_t i = 0; i < par->dest_bufsize; i++) {
		dest[i] = par->c_init;
	}

	st = &dest[par->dest_start];
	p = par->func(st, par->c_set, par->size);
	if (p != st) {
		fprintf(stderr, "return value of %s is invalid.\n", par->funcname);
		fflush(stderr);
		return -1;
	}

	r = verify_memset(par, dest);
	if (r) {
		return r;
	}

	free(dest);

	return 0;
}

int verify_memcpy_param(const struct param_memcpy *par)
{
	if (par->dest_bufsize < par->dest_start + par->size) {
		fprintf(stderr, "dest_bufsize:%d is too small "
			"(dest_start:%d + size:%d).\n",
			(int)par->dest_bufsize,
			(int)par->dest_start,
			(int)par->size);
		return -1;
	}

	if (par->src_bufsize < par->src_start + par->size) {
		fprintf(stderr, "src_bufsize:%d is too small "
			"(src_start:%d + size:%d).\n",
			(int)par->src_bufsize,
			(int)par->src_start,
			(int)par->size);
		return -1;
	}

	return 0;
}

int verify_memcpy(const struct param_memcpy *par, const char *dest, const char *src, const char *dest_org, const char *src_org)
{
	/* Check underflow */
	for (size_t i = 0; i < par->dest_start; i++) {
		if (dest[i] != dest_org[i]) {
			fprintf(stderr, "%s is dest underflow at %d. "
				"(expect:0x%x != actual:0x%x)\n",
				par->funcname,
				(int)i,
				dest_org[i] & 0xff,
				dest[i] & 0xff);
			return -1;
		}
	}

	for (size_t i = 0; i < par->src_start; i++) {
		if (src[i] != src_org[i]) {
			fprintf(stderr, "%s is src underflow at %d. "
				"(expect:0x%x != actual:0x%x)\n",
				par->funcname,
				(int)i,
				src_org[i] & 0xff,
				src[i] & 0xff);
			return -1;
		}
	}

	/* Check overflow */
	for (size_t i = par->dest_start + par->size; i < par->dest_bufsize; i++) {
		if (dest[i] != dest_org[i]) {
			fprintf(stderr, "%s is dest overflow at %d. "
				"(expect:0x%x != actual:0x%x)\n",
				par->funcname,
				(int)i,
				dest_org[i] & 0xff,
				dest[i] & 0xff);
			return -1;
		}
	}

	for (size_t i = par->src_start + par->size; i < par->src_bufsize; i++) {
		if (src[i] != src_org[i]) {
			fprintf(stderr, "%s is src overflow at %d. "
				"(expect:0x%x != actual:0x%x)\n",
				par->funcname,
				(int)i,
				src_org[i] & 0xff,
				src[i] & 0xff);
			return -1;
		}
	}

	/* Check memcpy */
	for (size_t i = 0; i < par->size; i++) {
		if (dest[par->dest_start + i] != src[par->src_start + i]) {
			fprintf(stderr, "%s failed to copy at %d. "
				"(dest:0x%x != src:0x%x)\n",
				par->funcname,
				(int)i,
				dest[par->dest_start + i] & 0xff,
				src[par->src_start + i] & 0xff);
			return -1;
		}
	}

	return 0;
}

int test_memcpy(const struct param_memcpy *par)
{
	char *dest = NULL, *src = NULL, *st_dest, *st_src;
	char *dest_org = NULL, *src_org = NULL;
	void *p;
	int r, ret = 0;

	r = verify_memcpy_param(par);
	if (r) {
		return r;
	}

	dest = malloc(par->dest_bufsize);
	if (dest == NULL) {
		perror("malloc(dest)");
		ret = -1;
		goto err_out;
	}

	dest_org = malloc(par->dest_bufsize);
	if (dest_org == NULL) {
		perror("malloc(dest_org)");
		ret = -1;
		goto err_out;
	}

	src = malloc(par->src_bufsize);
	if (src == NULL) {
		perror("malloc(src)");
		ret = -1;
		goto err_out;
	}

	src_org = malloc(par->src_bufsize);
	if (src_org == NULL) {
		perror("malloc(src_org)");
		ret = -1;
		goto err_out;
	}

	for (size_t i = 0; i < par->dest_bufsize; i++) {
		dest[i] = dest_org[i] = random();
	}

	for (size_t i = 0; i < par->src_bufsize; i++) {
		src[i] = src_org[i] = random();
	}

	st_dest = &dest[par->dest_start];
	st_src = &src[par->src_start];
	p = par->func(st_dest, st_src, par->size);
	if (p != st_dest) {
		fprintf(stderr, "return value of %s is invalid.\n", par->funcname);
		fflush(stderr);
		ret = -1;
		goto err_out;
	}

	r = verify_memcpy(par, dest, src, dest_org, src_org);
	if (r) {
		ret = r;
		goto err_out;
	}

err_out:
	free(src_org);
	free(src);
	free(dest_org);
	free(dest);

	return ret;
}

int bench_memset(size_t dstsize, int nloop, int kernel)
{
	struct timeval st, ed, el;
	char *dst = NULL;

	dst = malloc(dstsize);
	if (dst == NULL) {
		perror("malloc(dst)");
		return -1;
	}

	gettimeofday(&st, NULL);
	for (int i = 0; i < nloop; i++) {
		if (kernel) {
			kmemset(dst, 0, dstsize);
		} else {
			memset(dst, 0, dstsize);
		}
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &el);
	printf("  %d.%06d [s]\n", (int)el.tv_sec, (int)el.tv_usec);

	free(dst);

	return 0;
}

int bench_memcpy(size_t dstsize, int nloop, int kernel)
{
	struct timeval st, ed, el;
	char *dst = NULL;
	char *src = NULL;
	int res = 0;

	dst = malloc(dstsize);
	if (dst == NULL) {
		perror("malloc(dst)");
		res = -1;
		goto err_out;
	}

	src = malloc(dstsize);
	if (src == NULL) {
		perror("malloc(src)");
		res = -1;
		goto err_out;
	}

	gettimeofday(&st, NULL);
	for (int i = 0; i < nloop; i++) {
		if (kernel) {
			kmemcpy(dst, src, dstsize);
		} else {
			memcpy(dst, src, dstsize);
		}
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &el);
	printf("  %d.%06d [s]\n", (int)el.tv_sec, (int)el.tv_usec);

err_out:
	free(dst);
	free(src);

	return res;
}

int bench_strlen(size_t dstsize, int nloop, int kernel)
{
	struct timeval st, ed, el;
	volatile size_t len;
	char *s = NULL;
	int res = 0;

	s = malloc(dstsize);
	if (s == NULL) {
		perror("malloc(str)");
		return -1;
	}

	for (size_t i = 0; i < dstsize; i++) {
		s[i] = (i & 0xff) ? (i & 0xff) : 1;
	}
	s[dstsize - 1] = 0;

	gettimeofday(&st, NULL);
	for (int i = 0; i < nloop; i++) {
		if (kernel) {
			len = kstrlen(s);
		} else {
			len = strlen(s);
		}
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &el);
	printf("  %d.%06d [s]\n", (int)el.tv_sec, (int)el.tv_usec);

	if (len != dstsize - 1) {
		printf("len %d != dstsize-1 %d\n", (int)len, (int)(dstsize - 1));
		res = -1;
	}

	free(s);

	return res;
}

int main(int argc, char *argv[], char *envp[])
{
	struct timeval st, ed, el;
	int r, ret = 0;

	printf("%s: test crt_string start\n", argv[0]);

	printf("  memset: %d tests\n", (int)ARRAY_OF(par_kmemset));
	gettimeofday(&st, NULL);
	for (size_t i = 0; i < ARRAY_OF(par_kmemset); i++) {
		par_kmemset[i].funcname = "kmemset";
		par_kmemset[i].func = kmemset;
		r = test_memset(&par_kmemset[i]);
		if (r) {
			printf("  failed at %d\n", (int)i);
			ret = r;
			break;
		}
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &el);
	printf("  %d.%06d [s]\n", (int)el.tv_sec, (int)el.tv_usec);

	printf("  memcpy: %d tests\n", (int)ARRAY_OF(par_kmemcpy));
	gettimeofday(&st, NULL);
	for (size_t i = 0; i < ARRAY_OF(par_kmemcpy); i++) {
		par_kmemcpy[i].funcname = "kmemcpy";
		par_kmemcpy[i].func = kmemcpy;
		r = test_memcpy(&par_kmemcpy[i]);
		if (r) {
			printf("  failed at %d\n", (int)i);
			ret = r;
			break;
		}
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &el);
	printf("  %d.%06d [s]\n", (int)el.tv_sec, (int)el.tv_usec);

	/* benchmark */

	size_t bench_sz = 0x1000;
	int bench_cnt = 1000;

	printf("  kmemset bench: %d bytes x %d times\n", (int)bench_sz, bench_cnt);
	r = bench_memset(bench_sz, bench_cnt, 1);
	if (r) {
		printf("  failed\n");
		ret = r;
	}

	printf("  memset bench : %d bytes x %d times\n", (int)bench_sz, bench_cnt);
	r = bench_memset(bench_sz, bench_cnt, 0);
	if (r) {
		printf("  failed\n");
		ret = r;
	}

	printf("  kmemcpy bench: %d bytes x %d times\n", (int)bench_sz, bench_cnt);
	r = bench_memcpy(bench_sz, bench_cnt, 1);
	if (r) {
		printf("  failed\n");
		ret = r;
	}

	printf("  memcpy bench : %d bytes x %d times\n", (int)bench_sz, bench_cnt);
	r = bench_memcpy(bench_sz, bench_cnt, 0);
	if (r) {
		printf("  failed\n");
		ret = r;
	}

	printf("  kstrlen bench: %d bytes x %d times\n", (int)bench_sz, bench_cnt);
	r = bench_strlen(bench_sz, bench_cnt, 1);
	if (r) {
		printf("  failed\n");
		ret = r;
	}

	printf("  strlen bench : %d bytes x %d times\n", (int)bench_sz, bench_cnt);
	r = bench_strlen(bench_sz, bench_cnt, 0);
	if (r) {
		printf("  failed\n");
		ret = r;
	}


	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}
