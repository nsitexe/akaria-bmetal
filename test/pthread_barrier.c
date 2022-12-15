#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define THREADS    4
#define BUFSIZE    10

pthread_t th[THREADS];
pthread_barrier_t barrier;

int get_core_id(void)
{
	int ret = -1;
#if defined(__riscv)
	__asm volatile ("csrr %0, mhartid" : "=r"(ret));
#endif
	return ret;
}

void barrier_test_one(const char *n[], struct timeval *t)
{
	struct timeval start, end;
	int ind = 0;
        int r = 0;

	gettimeofday(&start, NULL);
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test_one wait_1\n");
            exit(1);
        }
	gettimeofday(&end, NULL);
	n[ind] = "0 prepare";
	timersub(&end, &start, &t[ind]);
	ind++;

	gettimeofday(&start, NULL);
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test_one wait_2\n");
            exit(1);
        }
	gettimeofday(&end, NULL);
	n[ind] = "1 barrier";
	timersub(&end, &start, &t[ind]);
	ind++;
}

void print_time(int n_threads, const char *variant, const char *pre, const char *n[], struct timeval *t)
{
	int i;

	for (i = 0; i < BUFSIZE; i++) {
		if (!n[i])
			break;

		printf("%2d %2s %2s: %20s: %2d: %d.%06d\n",
			n_threads, variant, pre, n[i], get_core_id(),
			(int)t[i].tv_sec, (int)t[i].tv_usec);
	}
}

void barrier_test(int n_threads, const char *variant)
{
	const char *names[BUFSIZE] = { 0, };
	struct timeval first[BUFSIZE], second[BUFSIZE], third[BUFSIZE];
        int r = 0;

	//1st: instructions are not cached
	barrier_test_one(names, first);

	//2nd, 3rd: cached
	barrier_test_one(names, second);
	barrier_test_one(names, third);

	printf("non-cached\n");
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test wait_1\n");
            exit(1);
        }
	print_time(n_threads, variant, "nc", names, first);
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test wait_2\n");
            exit(1);
        }

	printf("cached\n");
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test wait_3\n");
            exit(1);
        }
	print_time(n_threads, variant, "c1", names, second);
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test wait_4\n");
            exit(1);
        }
	printf("-----\n");
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test wait_5\n");
            exit(1);
        }
	print_time(n_threads, variant, "c2", names, third);
	r = pthread_barrier_wait(&barrier);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("NG : barrier_test wait_6\n");
            exit(1);
        }

	printf("completed\n");
}

void *thread_main(void *arg)
{
	barrier_test((intptr_t)arg, " ");

	return NULL;
}

int main(int argc, char *argv[], char *envp[])
{
	int cpuid = get_core_id();
	void *val;
	int r, ret = 0;

	printf("%s: test pthread_barrier\n", argv[0]);
	fflush(stdout);

	int n_threads = THREADS;

	r = pthread_barrier_init(&barrier, NULL, n_threads);
	if (r) {
		printf("%d: pthread_barrier_init: %s\n", cpuid, strerror(r));
		fflush(stdout);
	}

	for (int i = 0; i < n_threads - 1; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)n_threads);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	thread_main((void *)(intptr_t)n_threads);

	for (int i = 0; i < n_threads - 1; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}
	fflush(stdout);

	return ret;
}
