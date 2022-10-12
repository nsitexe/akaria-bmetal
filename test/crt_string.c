#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define ARRAY_OF(x)    (sizeof(x) / sizeof((x)[0]))

/* This is not POSIX application */
void *kmemset(void *s, int c, size_t n);
void *kmemcpy(void *dest, const void *src, size_t n);

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
	char *dest, *src, *st_dest, *st_src;
	char *dest_org, *src_org;
	void *p;
	int r;

	r = verify_memcpy_param(par);
	if (r) {
		return r;
	}

	dest = malloc(par->dest_bufsize);
	if (dest == NULL) {
		perror("malloc(dest)");
		return -1;
	}

	dest_org = malloc(par->dest_bufsize);
	if (dest_org == NULL) {
		perror("malloc(dest_org)");
		return -1;
	}

	src = malloc(par->src_bufsize);
	if (src == NULL) {
		perror("malloc(src)");
		return -1;
	}

	src_org = malloc(par->src_bufsize);
	if (src_org == NULL) {
		perror("malloc(src_org)");
		return -1;
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
		return -1;
	}

	r = verify_memcpy(par, dest, src, dest_org, src_org);
	if (r) {
		return r;
	}

	free(src_org);
	free(src);
	free(dest_org);
	free(dest);

	return 0;
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
			ret = r;
			break;
		}
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &el);
	printf("  %d.%06d [s]\n", (int)el.tv_sec, (int)el.tv_usec);

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}
