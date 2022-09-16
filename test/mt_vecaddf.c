#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if __riscv_vector == 1
#include <riscv_vector.h>

#define vecadd    vecadd_rvv
#else /* __riscv_vector == 1 */
#define vecadd    vecadd_scalar
#endif /* __riscv_vector == 1 */

#if __riscv_vector == 1
#endif /* __riscv_vector == 1 */

#define N               28
#define TEST_THREADS    3
#define MAX_THREADS     16

struct thread_arg {
	const float *a;
	const float *b;
	float *c;
	int off;
	int n;

	pthread_t th;
};

const float test_a[N] = {
	-10.21, -10.21, -10.21, -10.21, -10.21,
	-85.81, -85.81, -85.81, -85.81, -85.81,
	-100.001, -100.0001, -100.00001, -100.000001, -100.0000001,
	100.001, 100.0001, 100.00001, 100.000001, 100.0000001,
};

const float test_b[N] = {
	10.00, 10.20, 10.21, 10.30, 10.99,
	85.00, 85.80, 85.81, 85.90, 85.99,
	0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
	-0.001, -0.0001, -0.00001, -0.000001, -0.0000001,
};

float test_c[N];
float test_c_expect[N];

const int test_n = N;
const int test_t = TEST_THREADS;

struct thread_arg th_test_args[MAX_THREADS];

#if __riscv_vector == 1
void vecadd_rvv(const float *a, const float *b, float *c, int n)
{
	vfloat32m1_t va, vb, vc;
	size_t l;

	printf("----- use rvv f32\n");

	for (; n > 0; n -= l) {
		l = vsetvl_e32m1(n);
		va = vle32_v_f32m1(a, l);
		a += l;
		vb = vle32_v_f32m1(b, l);
		b += l;
		vc = vfadd_vv_f32m1(va, vb, l);
		vse32_v_f32m1(c, vc, l);
		c += l;
	}
}
#endif /* __riscv_vector == 1 */

void vecadd_scalar(const float *a, const float *b, float *c, int n)
{
	printf("----- use scalar f32\n");

	for (int i = 0; i < n; i++) {
		c[i] = a[i] + b[i];
	}
}

int fp_eq(float reference, float actual, float relErr)
{
	/* if near zero, do absolute error instead. */
	float absErr = relErr * ((fabsf(reference) > relErr) ? fabsf(reference) : relErr);
	return fabsf(actual - reference) < absErr;
}

void *thread_main(void *p)
{
	struct thread_arg *arg = p;

	vecadd(arg->a, arg->b, arg->c, arg->n);

	return NULL;
}

int main(int argc, char *argv[], char *envp[])
{
	const float *a, *b;
	float *c;
	const int *n, *t;
	int off, rem, check = 0, r;
	void *val;

	printf("%s: mt_vecadd start\n", argv[0]);

	printf("argc: %d\n", argc);
	if (argc > 4) {
		a = (const float *)argv[1];
		b = (const float *)argv[2];
		c = (float *)argv[3];
		n = (const int *)argv[4];
		t = (const int *)argv[5];
	} else {
		/* Use test data */
		a = test_a;
		b = test_b;
		c = test_c;
		n = &test_n;
		t = &test_t;
		check = 1;
	}

	printf("vector length: %d\n", *n);
	printf("threads: %d\n", *t);
	if (*t > MAX_THREADS) {
		printf("exceed max threads: %d\n", MAX_THREADS);
		return -1;
	}

	/* divide the area */
	off = 0;
	rem = *n;
	for (int i = 0; i < *t - 1; i++) {
		struct thread_arg *arg = &th_test_args[i];

		arg->off = off;
		arg->n = *n / *t;
		off += *n / *t;
		rem -= *n / *t;
	}
	th_test_args[*t - 1].off = off;
	th_test_args[*t - 1].n = rem;

	for (int i = 0; i < *t; i++) {
		struct thread_arg *arg = &th_test_args[i];

		printf("th %d: off:%d, n:%d\n", i, arg->off, arg->n);
		arg->a = a + arg->off;
		arg->b = b + arg->off;
		arg->c = c + arg->off;
	}

	/* create sub threads (index > 0) */
	for (int i = 1; i < *t; i++) {
		struct thread_arg *arg = &th_test_args[i];

		r = pthread_create(&arg->th, NULL, thread_main, arg);
		if (r) {
			printf("%d: pthread_create: %s\n", i, strerror(r));
			return r;
		}
	}

	/* main thread (index == 0) */
	thread_main(&th_test_args[0]);

	/* wait sub threads (index > 0) */
	for (int i = 1; i < *t; i++) {
		r = pthread_join(th_test_args[i].th, &val);
		if (r) {
			printf("%d: pthread_join: %s\n", i, strerror(r));
			return r;
		}
	}

	for (int i = 0; i < *n; i++) {
		if (i < 10) {
			printf("%d: a(%f) + b(%f) = c(%f)\n", i, a[i], b[i], c[i]);
		}
	}

	if (check) {
		int pass = 1;

		vecadd_scalar(a, b, test_c_expect, *n);

		for (int i = 0; i < N; i++) {
			if (!fp_eq(test_c_expect[i], c[i], 1e-6)) {
				printf("failed, %f=!%f\n", test_c_expect[i], c[i]);
				pass = 0;
			}
		}
		if (pass) {
			printf("passed\n");
		}
	}

	return 0;
}
