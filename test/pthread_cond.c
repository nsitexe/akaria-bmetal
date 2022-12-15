#define _GNU_SOURCE

#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define THREADS    4
#define LOOP_N     20000

pthread_t th[THREADS];
int n_threads = THREADS;
pthread_condattr_t cond_attr;
pthread_cond_t cond;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
int v_common = 0;

int get_core_id(void)
{
	int ret = -1;
#if defined(__riscv)
	__asm volatile ("csrr %0, mhartid" : "=r"(ret));
#endif
	return ret;
}

int get_thread_id(void)
{
	int ret = -1;
#if defined(HAVE_PTHREAD_GETTHREADID_NP)
	ret = pthread_getthreadid_np();
#elif defined(__linux)
	ret = gettid();
#endif
	return ret;
}

void *thread_main(void *arg)
{
	int cpuid = get_core_id();
	int tid = get_thread_id();
	int v = (int)(intptr_t)arg;
	int v_s = v * LOOP_N, v_e = (v + 1) * LOOP_N;
	int v_fin = n_threads * LOOP_N;

	printf("%d: tid:%d: step1-1 prep %05d-%05d\n", cpuid, tid, v_s, v_e);
	fflush(stdout);

	pthread_mutex_lock(&cond_mutex);
	while (v_common < v_s) {
		pthread_cond_wait(&cond, &cond_mutex);
	}
	pthread_mutex_unlock(&cond_mutex);

	printf("%d: tid:%d: step1-2 cnt  %05d-%05d \n", cpuid, tid, v_s, v_e);
	fflush(stdout);

	while (1) {
		pthread_mutex_lock(&cond_mutex);
		if (v_common >= v_e) {
			pthread_mutex_unlock(&cond_mutex);
			break;
		}
		v_common++;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&cond_mutex);
	}

	printf("%d: tid:%d: step1-3 wait %05d-%05d\n", cpuid, tid, v_s, v_e);
	fflush(stdout);

	pthread_mutex_lock(&cond_mutex);
	while (v_common < v_fin) {
		pthread_cond_wait(&cond, &cond_mutex);
	}
	pthread_mutex_unlock(&cond_mutex);

	printf("%d: tid:%d: step1-4 fin  %05d-%05d\n", cpuid, tid, v_s, v_e);
	fflush(stdout);

	return NULL;
}

int main(int argc, char *argv[], char *envp[])
{
	int cpuid = get_core_id();
	void *val;
	int r, ret = 0;

	printf("%s: test pthread_cond\n", argv[0]);
	fflush(stdout);

	r = pthread_condattr_init(&cond_attr);
	if (r) {
		printf("%d: pthread_condattr_init: %s\n", cpuid, strerror(r));
		ret = r;
		goto err_out;
	}

	r = pthread_cond_init(&cond, &cond_attr);
	if (r) {
		printf("%d: pthread_cond_init: %s\n", cpuid, strerror(r));
		ret = r;
		goto err_out;
	}

	for (int i = 0; i < n_threads - 1; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	thread_main((void *)(intptr_t)(n_threads - 1));

	for (int i = 0; i < n_threads - 1; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

err_out:
	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}
	fflush(stdout);

	return ret;
}
