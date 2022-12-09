#define _GNU_SOURCE

#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define LOOP_N    300000
int __arch_riscv_get_cpu_id(void);

pthread_attr_t attr;
pthread_t th[20];
pthread_barrier_t barrier1;
pthread_barrier_t barrier2;
int n_threads = 3;

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
	struct timeval st, ed, elapse;
	int cpuid = __arch_riscv_get_cpu_id();
	int v = (int)(intptr_t)arg * 10 + 1;

	printf("%d: ---- thread step1 arg:%d %p, pid:%d, tid:%d\n", cpuid, v, &arg, getpid(), get_thread_id());
	fflush(stdout);

	printf("%d: ---- thread step2 st arg:%d %p\n", cpuid, v, &arg);
	gettimeofday(&st, NULL);
	for (int i = 0; i < LOOP_N; i++) {
		fflush(stdout);
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &elapse);
	printf("%d: ---- thread step2 ed arg:%d %p elapsed: %d.%06d[s] loop:%d\n",
		cpuid, v, &arg, (int)elapse.tv_sec, (int)elapse.tv_usec, LOOP_N);
	fflush(stdout);

	printf("%d: ---- thread step3 arg:%d %p\n", cpuid, v, &arg);
	fflush(stdout);

	return NULL;
}

void *parent_thread_main(void *arg)
{
	int cpuid = __arch_riscv_get_cpu_id();
	int v = (int)(intptr_t)arg;
	void *val, *ret;
	int st, r;

	st = v + 1;
	for (int i = st; i < st + n_threads - 1; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	if (n_threads >= 2) {
		r = pthread_join(th[st], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	ret = thread_main(arg);

	for (int i = st + 1; i < st + n_threads - 1; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	return ret;
}

void *thread_barrier_main(void *arg)
{
	struct timeval st, ed, elapse;
	int cpuid = __arch_riscv_get_cpu_id();
	int v = (int)(intptr_t)arg * 10 + 1;
	int r;

	printf("%d: ---- thread step1 arg:%d %p, pid:%d, tid:%d\n", cpuid, v, &arg, getpid(), get_thread_id());
	fflush(stdout);

	printf("%d: ---- thread step2 st arg:%d %p\n", cpuid, v, &arg);
	gettimeofday(&st, NULL);
	for (int i = 0; i < LOOP_N; i++) {
		fflush(stdout);
	}
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &elapse);
	printf("%d: ---- thread step2 ed arg:%d %p elapsed: %d.%06d[s] loop:%d\n",
		cpuid, v, &arg, (int)elapse.tv_sec, (int)elapse.tv_usec, LOOP_N);
	fflush(stdout);

	printf("%d: ---- thread step3 arg:%d %p\n", cpuid, v, &arg);
	fflush(stdout);

	r = pthread_barrier_wait(&barrier1);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("%d: pthread_barrier_wait: 1 %s\n", cpuid, strerror(r));
	}

	r = pthread_barrier_wait(&barrier2);
	if (r && r != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("%d: pthread_barrier_wait: 2 %s\n", cpuid, strerror(r));
	}

	printf("%d: ---- thread step4 arg:%d %p\n", cpuid, v, &arg);
	fflush(stdout);

	return NULL;
}

int main(int argc, char *argv[], char *envp[])
{
	int cpuid = __arch_riscv_get_cpu_id();
	void *val;
	int st = 0, r, ret = 0;

	printf("%s: test pthread\n", argv[0]);
	fflush(stdout);

	printf("%d: pid:%d, tid:%d\n", cpuid, getpid(), get_thread_id());
	fflush(stdout);

	r = pthread_attr_init(&attr);
	if (r) {
		printf("%d: pthread_attr_init: %s\n", cpuid, strerror(r));
		ret = r;
	}

	r = pthread_attr_setstacksize(&attr, 32768);
	if (r) {
		printf("%d: pthread_attr_setstacksize: %s\n", cpuid, strerror(r));
	}
	fflush(stdout);

	printf("%d: -------- step1-1 main + 1threads start\n", cpuid);
	fflush(stdout);

	if (n_threads >= 1) {
		for (int i = st; i < st + 1; i++) {
			r = pthread_create(&th[i], &attr, thread_main, (void *)(intptr_t)i);
			if (r) {
				printf("%d: pthread_create: %s\n", cpuid, strerror(r));
				ret = r;
			}
			fflush(stdout);
		}

		thread_main((void *)(intptr_t)(st + 1));

		for (int i = st; i < st + 1; i++) {
			r = pthread_join(th[i], &val);
			if (r) {
				printf("%d: pthread_join: %s\n", cpuid, strerror(r));
				ret = r;
			}
			fflush(stdout);
		}

		st += 1 + 1;
	} else {
		printf("%d: -------- skipped.\n", cpuid);
	}

	printf("%d: -------- step1-1 joined\n", cpuid);
	fflush(stdout);


	printf("%d: -------- step1-2 main + 2threads start\n", cpuid);
	fflush(stdout);

	if (n_threads >= 2) {
		for (int i = st; i < st + 2; i++) {
			r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
			if (r) {
				printf("%d: pthread_create: %s\n", cpuid, strerror(r));
				ret = r;
			}
			fflush(stdout);
		}

		thread_main((void *)(intptr_t)(st + 2));

		for (int i = st; i < st + 2; i++) {
			r = pthread_join(th[i], &val);
			if (r) {
				printf("%d: pthread_join: %s\n", cpuid, strerror(r));
				ret = r;
			}
			fflush(stdout);
		}

		st += 1 + 2;
	} else {
		printf("%d: -------- skipped.\n", cpuid);
	}

	printf("%d: -------- step1-2 joined\n", cpuid);
	fflush(stdout);


	printf("%d: -------- step2 main + all start\n", cpuid);
	fflush(stdout);

	for (int i = st; i < st + n_threads; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	thread_main((void *)(intptr_t)(st + n_threads));

	for (int i = st; i < st + n_threads; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	st += 1 + n_threads;

	printf("%d: -------- step2 joined\n", cpuid);
	fflush(stdout);


	printf("%d: -------- step3 main + parent + all start\n", cpuid);
	fflush(stdout);

	if (n_threads >= 1) {
		r = pthread_create(&th[st], NULL, parent_thread_main, (void *)(intptr_t)st);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);

		thread_main((void *)(intptr_t)(st + n_threads));

		r = pthread_join(th[st], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);

		st += 1 + n_threads;
	} else {
		printf("-------- skipped.\n");
	}

	printf("%d: -------- step3 joined\n", cpuid);
	fflush(stdout);


	printf("%d: -------- step4 main + all (barrier) start\n", cpuid);
	fflush(stdout);

	r = pthread_barrier_init(&barrier1, NULL, n_threads + 1);
	if (r) {
		printf("%d: pthread_barrier_init: 1 %s\n", cpuid, strerror(r));
		ret = r;
	}

	r = pthread_barrier_init(&barrier2, NULL, n_threads + 1);
	if (r) {
		printf("%d: pthread_barrier_init: 2 %s\n", cpuid, strerror(r));
		ret = r;
	}

	for (int i = st; i < st + n_threads; i++) {
		r = pthread_create(&th[i], NULL, thread_barrier_main, (void *)(intptr_t)i);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	thread_barrier_main((void *)(intptr_t)(st + n_threads));

	for (int i = st; i < st + n_threads; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	st += 1 + n_threads;

	printf("%d: -------- step4 joined\n", cpuid);
	fflush(stdout);


	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}
	fflush(stdout);

	return ret;
}
