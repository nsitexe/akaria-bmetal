#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#define mcycle     0xb00
#define mcycleh    0xb80

#define N_TESTS    23

#define PI    3.1415926535897f

typedef int (*testfunc_t)(int);

struct testcase {
	const char *name;
	testfunc_t func;
	float val;

	uint64_t elapse;
};

/* avoid to optimize */
float g_f;
float g_f2;

uint64_t get_clk(void)
{
#ifdef __riscv
	uint32_t h, l;

	__asm volatile("csrr %0, %2\n"
		"csrr %1, %3\n"
		: "=r"(l), "=r"(h)
		: "i"(mcycle), "i"(mcycleh));

	return (((uint64_t)h) << 32) | l;
#else
	return 0;
#endif
}

int case_nothing(int id)
{
	return 0;
}

int case_ldst(int id)
{
	g_f = 1.1f;
	g_f2 = 1.2f;

	return 0;
}

int case_addf(int id)
{
	g_f += 10.1f;
	g_f2 = g_f;

	return 0;
}

int case_mulf(int id)
{
	g_f *= 100.1f;
	g_f2 = g_f;

	return 0;
}

int case_divf(int id)
{
	g_f /= 1000.1f;
	g_f2 = g_f;

	return 0;
}

int case_sqrtf(int id)
{
	g_f = sqrtf(g_f);
	g_f2 = g_f;

	return 0;
}

int case_exp2f(int id)
{
	g_f = exp2f(g_f);
	g_f2 = g_f;

	return 0;
}

int case_expf(int id)
{
	g_f = expf(g_f);
	g_f2 = g_f;

	return 0;
}

int case_logf(int id)
{
	g_f = logf(g_f);
	g_f2 = g_f;

	return 0;
}

int case_log2f(int id)
{
	g_f = exp2f(g_f);
	g_f2 = g_f;

	return 0;
}

int case_sinf(int id)
{
	g_f = sinf(g_f);
	g_f2 = g_f;

	return 0;
}

int case_cosf(int id)
{
	g_f = cosf(g_f);
	g_f2 = g_f;

	return 0;
}

int case_tanf(int id)
{
	g_f = tanf(g_f);
	g_f2 = g_f;

	return 0;
}

struct testcase tests_org[N_TESTS] = {
	{"nothing1", case_nothing},
	{"nothing2", case_nothing},
	{"ldst", case_ldst},
	{"addf", case_addf},
	{"mulf", case_mulf},
	{"divf", case_divf},
	{"sqrtf", case_sqrtf},
	{"exp2f", case_exp2f, 1000.0f},
	{"expf",  case_expf,  1000.0f},
	{"log2f", case_log2f, 100000.0f},
	{"logf",  case_logf,  100000.0f},
	{"sinf(small)",  case_sinf, PI / 4},
	{"sinf(medium)", case_sinf, 5 * PI / 4},
	{"sinf(large)",  case_sinf, 192 * PI / 4},
	{"sinf(vlarge)", case_sinf, 1234},
	{"cosf(small)",  case_cosf, PI / 4},
	{"cosf(medium)", case_cosf, 5 * PI / 4},
	{"cosf(large)",  case_cosf, 192 * PI / 4},
	{"cosf(vlarge)", case_cosf, 1234},
	{"tanf(small)",  case_tanf, PI / 4},
	{"tanf(medium)", case_tanf, 5 * PI / 4},
	{"tanf(large)",  case_tanf, 192 * PI / 4},
	{"tanf(vlarge)", case_tanf, 1234},
};

struct testcase tests_nc[N_TESTS];
struct testcase tests_c[N_TESTS];

void show_result(struct testcase *tests)
{
	for (int i = 0; i < N_TESTS; i++) {
		printf("%20s: %d[clk]\n", tests[i].name, (int)tests[i].elapse);
	}
	fflush(stdout);
}

int test_float(struct testcase *tests)
{
	for (int i = 0; i < N_TESTS; i++) {
		int r;

		if (tests[i].func) {
			uint64_t st, ed;

			g_f = tests[i].val;

			st = get_clk();
			r = tests[i].func(i);
			ed = get_clk();
			tests[i].elapse = ed - st;

			if (r != 0) {
				fprintf(stderr, "failed test '%s' (id:%d).\n", tests[i].name, i);
				return -1;
			}
		}
	}


	return 0;
}

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: test float start\n", argv[0]);

	memcpy(tests_nc, tests_org, sizeof(tests_nc));
	memcpy(tests_c, tests_org, sizeof(tests_c));

	r = test_float(tests_nc);
	if (r) {
		printf("%s: test_float(non-cached) failed.\n", argv[0]);
		ret = r;
	}

	r = test_float(tests_c);
	if (r) {
		printf("%s: test_float(cached) failed.\n", argv[0]);
		ret = r;
	}

	printf("non-cached -----\n");
	show_result(tests_nc);
	printf("cached -----\n");
	show_result(tests_c);

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return ret;
}
