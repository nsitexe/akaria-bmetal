/* Please define this macro to use CBLAS to generate expected values */
//#define USE_CBLAS

/* Please define this macro to show debug messages */
#define DEBUG_PRINT

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

#ifdef USE_CBLAS
#include <cblas.h>
#endif

#define M    19
#define N    17
#define K    23

float ta[M * K];
float tb[K * N];
float tc[M * N];

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
void sgemm_rvv(const float *a, const float *b, float *c, int mm, int nn, int kk)
{
	vfloat32m1_t va, vb, vc, vt;
	size_t vl;

	dbgprintf("----- use rvv f32 inner\n");

	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			c[i * nn + j] = 0.0f;
			for (int k = 0; k < kk; k += vl) {
				vl = vsetvl_e32m1(kk - k);
				va = vle32_v_f32m1(&a[i * kk + k], vl);
				vb = vlse32_v_f32m1(&b[k * nn + j], nn * sizeof(float), vl);
				vc = vle32_v_f32m1(&c[i * nn + j], 1);

				vt = vfmul_vv_f32m1(va, vb, vl);
				vc = vfredusum_vs_f32m1_f32m1(vc, vt, vc, vl);
				c[i * nn + j] = vfmv_f_s_f32m1_f32(vc);
			}
		}
	}
}
#endif /* __riscv_vector == 1 */

void sgemm_scalar(const float *a, const float *b, float *c, int mm, int nn, int kk)
{
	dbgprintf("----- use scalar f32\n");

	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			c[i * nn + j] = 0.0f;
			for (int k = 0; k < kk; k++) {
				c[i * nn + j] += a[i * kk + k] * b[k * nn + j];
			}
		}
	}
}

void sgemm_naive(const float *a, const float *b, float *c, int mm, int nn, int kk)
{
	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			c[i * nn + j] = 0.0f;
			for (int k = 0; k < kk; k++) {
				c[i * nn + j] += a[i * kk + k] * b[k * nn + j];
			}
		}
	}
}

void init_test(float *a, float *b, float *c, int mm, int nn, int kk)
{
	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < kk; j++) {
			a[i * kk + j] = (float)(i * j + 10) / (mm * kk);
		}
	}
	for (int i = 0; i < kk; i++) {
		for (int j = 0; j < nn; j++) {
			b[i * nn + j] = (float)(i + j + 10) / (kk + nn);
		}
	}
	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			c[i * nn + j] = -1.0f;
		}
	}
}

int fp_eq(float reference, float actual, float relErr)
{
	/* if near zero, do absolute error instead. */
	float absErr = relErr * ((fabsf(reference) > relErr) ? fabsf(reference) : relErr);
	return fabsf(actual - reference) < absErr;
}

int check_result(const float *c, const float *c_ex, int l, int m, int n)
{
	int cnt = 0;

	for (int i = 0; i < l; i++) {
		for (int j = 0; j < m; j++) {
			if (!fp_eq(c_ex[i * m + j], c[i * m + j], 1e-6)) {
				if (cnt >= 10) {
					continue;
				}

				printf("[%d, %d]: %f != %f (expected)\n", i, j,
					c[i * m + j], c_ex[i * m + j]);
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
	int check = 0, m, n, k, r;

	printf("%s: sgemm start\n", argv[0]);

	dbgprintf("argc: %d\n", argc);
	if (argc > 4) {
		a = (const float *)argv[1];
		b = (const float *)argv[2];
		c = (float *)argv[3];
		m = *(int *)argv[4];
		n = *(int *)argv[4];
		k = *(int *)argv[4];
	} else {
		init_test(ta, tb, tc, M, N, K);

		a = (const float *)ta;
		b = (const float *)tb;
		c = (float *)tc;
		m = M;
		n = N;
		k = K;
		check = 1;
	}

	dbgprintf("matrix size: M:%d, N:%d, K:%d\n", m, n, k);
	gettimeofday(&st, NULL);
	sgemm(a, b, c, m, n, k);
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &ela);

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			if (i < 4 && j < 4) {
				dbgprintf("[%d, %d]: a(%f), b(%f), c(%f)\n", i, j,
					a[i * k + j], b[i * n + j], c[i * n + j]);
			}
		}
	}
	printf("time: %d.%06d\n", (int)ela.tv_sec, (int)ela.tv_usec);

	if (check) {
		float *c_ex;
		int pass = 1;

		c_ex = malloc(m * n * sizeof(float));

#ifdef USE_CBLAS
		// C = alpha AB + beta C
		float alpha = 1.0f, beta = 0.0f;
		int lda = k, ldb = n, ldc = n;

		dbgprintf("use CBLAS\n");
		cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
			m, n, k, alpha, a, lda, b, ldb, beta, c_ex, ldc);
#else
		sgemm_naive(a, b, c_ex, m, n, k);
#endif
		r = check_result(c, c_ex, m, n, k);
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
