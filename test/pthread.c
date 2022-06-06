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
pthread_t th[8];

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
	int r;

	for (int i = 1; i < 3; i++) {
		r = pthread_create(&th[v + i], NULL, thread_main, (void *)(intptr_t)(v + i));
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	r = pthread_join(th[v + 1], &val);
	if (r) {
		printf("%d: pthread_join: %s\n", cpuid, strerror(r));
	}
	fflush(stdout);

	ret = thread_main(arg);

	r = pthread_join(th[v + 2], &val);
	if (r) {
		printf("%d: pthread_join: %s\n", cpuid, strerror(r));
	}
	fflush(stdout);

	return ret;
}

int main(int argc, char *argv[], char *envp[])
{
	int cpuid = __arch_riscv_get_cpu_id();
	void *val;
	int r;

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

	printf("%d: -------- step1 2threads start\n", cpuid);
	for (int i = 0; i < 2; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	for (int i = 0; i < 2; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}
	printf("%d: -------- step1 joined\n", cpuid);
	fflush(stdout);

	printf("%d: -------- step2 3threads start\n", cpuid);
	fflush(stdout);
	for (int i = 2; i < 5; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
		if (r) {
			printf("%d: pthread_create: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}

	for (int i = 2; i < 5; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", cpuid, strerror(r));
		}
		fflush(stdout);
	}
	printf("%d: -------- step2 joined\n", cpuid);
	fflush(stdout);

	printf("%d: -------- step3 thread from thread start\n", cpuid);
	fflush(stdout);
	r = pthread_create(&th[5], NULL, parent_thread_main, (void *)(intptr_t)5);
	if (r) {
		printf("%d: pthread_create: %s\n", cpuid, strerror(r));
	}
	fflush(stdout);

	r = pthread_join(th[5], &val);
	if (r) {
		printf("%d: pthread_join: %s\n", cpuid, strerror(r));
	}
	fflush(stdout);

	printf("%d: -------- step3 joined\n", cpuid);
	fflush(stdout);

	return 0;
}
