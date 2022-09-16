#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cmn_vecaddf.h"

#define N               128
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

	dbgprintf("argc: %d\n", argc);
	if (argc > 5) {
		t = (const int *)argv[5];
	} else {
		t = &test_t;
	}
	if (argc > 4) {
		a = (const float *)argv[1];
		b = (const float *)argv[2];
		c = (float *)argv[3];
		n = (const int *)argv[4];
	} else {
		/* Use test data */
		a = test_a;
		b = test_b;
		c = test_c;
		n = &test_n;
		check = 1;
	}

	dbgprintf("vector length: %d\n", *n);
	dbgprintf("threads: %d\n", *t);
	if (*t <= 0 || MAX_THREADS < *t) {
		printf("invalid or exceed max threads:%d\n", MAX_THREADS);
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

		dbgprintf("th %d: off:%d, n:%d\n", i, arg->off, arg->n);
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
			dbgprintf("%d: a(%f) + b(%f) = c(%f)\n", i, a[i], b[i], c[i]);
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
			dbgprintf("passed\n");
		}
	}

	return 0;
}
