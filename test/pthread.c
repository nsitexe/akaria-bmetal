#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define NOP    __asm volatile("nop")

uint64_t get_cycle(void)
{
	uint64_t v;
	const int mcycle = 0xb00;

	__asm volatile("csrr %0, %1" : "=r"(v) : "i"(mcycle));

	return v;
}

int test_args(int argc, char *argv[])
{
	if (argc <= 0) {
		printf("argc is zero.\n");
		return -1;
	}
	if (argv[0] == NULL) {
		printf("argv[0] is NULL.\n");
		return -1;
	}

	return 0;
}

int test_ctype(void)
{
	volatile char ch_a = 'a';
	volatile char ch_cb = 'B';

	if (isupper(ch_a)) {
		printf("isupper('%c') result is incorrect.\n", ch_a);
		return -1;
	}
	if (islower(ch_cb)) {
		printf("islower('%c') result is incorrect.\n", ch_cb);
		return -1;
	}

	return 0;
}

void *thread_main(void *arg)
{
	printf("thread!! argh:%d\n", (int)(intptr_t)arg);
	fflush(stdout);

	return NULL;
}

int main(int argc, char *argv[], char *envp[])
{
	printf("%s: hello world!\n", argv[0]);

	printf("pid:%d\n", getpid());
	fflush(stdout);

	if (test_args(argc, argv)) {
		printf("test_args failed.\n");
	}
	if (test_ctype()) {
		printf("%s: test_ctype failed.\n", argv[0]);
	}

	volatile uint64_t s = get_cycle();
	volatile uint64_t e = get_cycle();
	printf("clock1! %"PRIx64"\n", s);
	printf("clock1! %"PRIx64"\n", e);

	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %p\n", i, argv[i]);
	}
	printf("clock2! %"PRIx64"\n", get_cycle());
	fflush(stdout);

	pthread_t th[4];
	pthread_attr_t attr;
	int r;

	r = pthread_attr_init(&attr);
	if (r) {
		printf("pthread_attr_init: %s\n", strerror(r));
	}

	//r = pthread_attr_setstacksize(&attr, 32768);
	//if (r) {
	//	printf("pthread_attr_setstacksize: %s\n", strerror(r));
	//}
	//fflush(stdout);

	r = pthread_create(&th[0], NULL, thread_main, (void *)11);
	if (r) {
		printf("pthread_create: %s\n", strerror(r));
	}
	fflush(stdout);

	r = pthread_create(&th[1], NULL, thread_main, (void *)21);
	if (r) {
		printf("pthread_create: %s\n", strerror(r));
	}
	fflush(stdout);

	for (int i = 0; i < 2; i++) {
		void *val;

		r = pthread_join(th[i], &val);
		if (r) {
			printf("pthread_join: %s\n", strerror(r));
		}
		fflush(stdout);
	}

	r = pthread_create(&th[2], NULL, thread_main, (void *)31);
	if (r) {
		printf("pthread_create: %s\n", strerror(r));
	}
	fflush(stdout);

	r = pthread_create(&th[3], NULL, thread_main, (void *)41);
	if (r) {
		printf("pthread_create: %s\n", strerror(r));
	}
	fflush(stdout);

	for (int i = 2; i < 4; i++) {
		void *val;

		r = pthread_join(th[i], &val);
		if (r) {
			printf("pthread_join: %s\n", strerror(r));
		}
		fflush(stdout);
	}

	return 0;
}
