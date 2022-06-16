#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int __arch_riscv_get_cpu_id(void);

pthread_attr_t attr;
pthread_t th[12];
int n_threads = 3;

void *thread_main(void *arg)
{
	int cpuid = __arch_riscv_get_cpu_id();
	int v = (int)(intptr_t)arg * 10 + 1;

	printf("%d: ---- thread step1 arg:%d %p\n", cpuid, v, &arg);
	fflush(stdout);
	printf("%d: ---- thread step2 arg:%d %p\n", cpuid, v, &arg);
	for (int i = 0; i < 1000000; i++) {
		fflush(stdout);
	}
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

int main(int argc, char *argv[], char *envp[])
{
	int cpuid = __arch_riscv_get_cpu_id();
	void *val;
	int st = 0, r;

	printf("%s: test pthread\n", argv[0]);
	fflush(stdout);

	r = pthread_attr_init(&attr);
	if (r) {
		printf("%d: pthread_attr_init: %s\n", cpuid, strerror(r));
	}

	//r = pthread_attr_setstacksize(&attr, 32768);
	//if (r) {
	//	printf("%d: pthread_attr_setstacksize: %s\n", cpuid, strerror(r));
	//}
	//fflush(stdout);

	printf("%d: -------- step1-1 1threads start\n", cpuid);
	fflush(stdout);

	if (n_threads >= 1) {
		for (int i = st; i < st + 1; i++) {
			r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
			if (r) {
				printf("%d: pthread_create: %s\n", cpuid, strerror(r));
			}
			fflush(stdout);
		}

		for (int i = st; i < st + 1; i++) {
			r = pthread_join(th[i], &val);
			if (r) {
				printf("%d: pthread_join: %s\n", cpuid, strerror(r));
			}
			fflush(stdout);
		}

		st += 1;
	} else {
		printf("%d: -------- skipped.\n", cpuid);
	}

	printf("%d: -------- step1-1 joined\n", cpuid);
	fflush(stdout);


	printf("%d: -------- step1-2 2threads start\n", cpuid);
	fflush(stdout);

	if (n_threads >= 2) {
		for (int i = st; i < st + 2; i++) {
			r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
			if (r) {
				printf("%d: pthread_create: %s\n", cpuid, strerror(r));
			}
			fflush(stdout);
		}

		for (int i = st; i < st + 2; i++) {
			r = pthread_join(th[i], &val);
			if (r) {
				printf("%d: pthread_join: %s\n", cpuid, strerror(r));
			}
			fflush(stdout);
		}

		st += 2;
	} else {
		printf("%d: -------- skipped.\n", cpuid);
	}

	printf("%d: -------- step1-2 joined\n", cpuid);
	fflush(stdout);


	printf("%d: -------- step2 all threads start\n", cpuid);
	fflush(stdout);

	for (int i = st; i < st + n_threads; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	for (int i = st; i < st + n_threads; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	st += n_threads;

	printf("%d: -------- step2 joined\n", cpuid);
	fflush(stdout);


	printf("%d: -------- step3 thread from thread start\n", cpuid);
	fflush(stdout);

	if (n_threads >= 1) {
		r = pthread_create(&th[st], NULL, parent_thread_main, (void *)(intptr_t)st);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);

		r = pthread_join(th[st], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);

		st = st + n_threads;
	} else {
		printf("-------- skipped.\n");
	}

	printf("%d: -------- step3 joined\n", cpuid);
	fflush(stdout);

	return 0;
}
