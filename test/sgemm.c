#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#if __riscv_vector == 1
#include <riscv_vector.h>

#define sgemm    sgemm_rvv
#else /* __riscv_vector == 1 */
#define sgemm    sgemm_scalar
#endif /* __riscv_vector == 1 */

#define N    17

float ta[N * N];
float tb[N * N];
float tc[N * N];

/* Please define this macro to show debug messages */
#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define dbgprintf    printf
#else /* DEBUG_PRINT */
#define dbgprintf    noprintf
#endif /* DEBUG_PRINT */

static inline int __attribute__((format(printf, 1, 2))) noprintf(const char *format, ...)
{
	return 0;
}

#if __riscv_vector == 1
void sgemm_rvv(const float *a, const float *b, float *c, int n)
{
	vfloat32m1_t va, vb, vc, vt;
	size_t l;

	dbgprintf("----- use rvv f32\n");

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			c[i * n + j] = 0.0f;
			for (int k = 0; k < n; k += l) {
				l = vsetvl_e32m1(n - k);
				va = vle32_v_f32m1(&a[i * n + k], l);
				vb = vlse32_v_f32m1(&b[k * n + j], n * sizeof(float), l);
				vc = vle32_v_f32m1(&c[i * n + j], 1);

				vt = vfmul_vv_f32m1(va, vb, l);
				vc = vfredusum_vs_f32m1_f32m1(vc, vt, vc, l);
				vse32_v_f32m1(&c[i * n + j], vc, 1);
			}
		}
	}
}
#endif /* __riscv_vector == 1 */

void sgemm_scalar(const float *a, const float *b, float *c, int n)
{
	dbgprintf("----- use scalar f32\n");

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			c[i * n + j] = 0.0f;
			for (int k = 0; k < n; k++) {
				c[i * n + j] += a[i * n + k] * b[k * n + j];
			}
		}
	}
}

void sgemm_naive(const float *a, const float *b, float *c, int n)
{
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			c[i * n + j] = 0.0f;
			for (int k = 0; k < n; k++) {
				c[i * n + j] += a[i * n + k] * b[k * n + j];
			}
		}
	}
}

void init_test(float *a, float *b, int n)
{
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			a[i * n + j] = (float)(i * j) / n;
			b[i * n + j] = (float)(i + j) / n;
		}
	}
}

int fp_eq(float reference, float actual, float relErr)
{
	/* if near zero, do absolute error instead. */
	float absErr = relErr * ((fabsf(reference) > relErr) ? fabsf(reference) : relErr);
	return fabsf(actual - reference) < absErr;
}

int check_result(const float *c, const float *c_ex, int n)
{
	int cnt = 0;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (!fp_eq(c_ex[i * n + j], c[i * n + j], 1e-6)) {
				if (cnt >= 10) {
					continue;
				}

				printf("[%d, %d]: %f != %f (expected)\n", i, j,
					c[i * n + j], c_ex[i * n + j]);
				cnt++;
			}
		}
	}

	return cnt;
}

int main(int argc, char *argv[], char *envp[])
{
	struct timeval st, ed, ela;
	const float *a, *b;
	float *c;
	int check = 0, n, r;

	printf("%s: sgemm start\n", argv[0]);

	dbgprintf("argc: %d\n", argc);
	if (argc > 4) {
		a = (const float *)argv[1];
		b = (const float *)argv[2];
		c = (float *)argv[3];
		n = *(int *)argv[4];
	} else {
		init_test(ta, tb, N);

		a = (const float *)ta;
		b = (const float *)tb;
		c = (float *)tc;
		n = N;
		check = 1;
	}

	dbgprintf("matrix size: %d\n", n);
	gettimeofday(&st, NULL);
	sgemm(a, b, c, n);
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &ela);

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (i < 4 && j < 4) {
				dbgprintf("[%d, %d]: a(%f), b(%f), c(%f)\n", i, j,
					a[i * n + j], b[i * n + j], c[i * n + j]);
			}
		}
	}
	printf("time: %d.%06d\n", (int)ela.tv_sec, (int)ela.tv_usec);

	if (check) {
		float *c_ex;
		int pass = 1;

		c_ex = malloc(n * n * sizeof(float));

		sgemm_naive(a, b, c_ex, n);
		r = check_result(c, c_ex, n);
		if (r) {
			pass = 0;
		}

		if (pass) {
			printf("passed\n");
		}

		free(c_ex);
	}

	return 0;
}
