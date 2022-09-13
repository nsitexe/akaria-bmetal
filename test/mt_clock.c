#define _GNU_SOURCE

#include <ctype.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef timespecsub
#define timespecsub(a, b, res)                                \
	do {                                                  \
		(res)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
		(res)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
		if ((res)->tv_nsec < 0) {                     \
			(res)->tv_sec -= 1;                   \
			(res)->tv_nsec += 1000000000L;        \
		}                                             \
	} while (0)
#endif /* timespecsub */

#ifndef timespecclear
#define timespecclear(tsp)          \
	do {                        \
		(tsp)->tv_sec = 0;  \
		(tsp)->tv_nsec = 0; \
	} while (0)
#endif /* timespecclear */

pthread_t th[12];
int n_threads = 4;
int n_poll = 5;

#ifdef _POSIX_TIMERS
int test_clock_poll(int tid)
{
	struct timespec st, ed, elapse;

	for (int i = 0; i < n_poll; i++) {
		int j = 0;
		clock_gettime(CLOCK_REALTIME, &st);
		timespecclear(&elapse);

		while (elapse.tv_sec < 2) {
			clock_gettime(CLOCK_REALTIME, &ed);
			timespecsub(&ed, &st, &elapse);
			j++;
		}

		printf("tid %3d: %d: poll clock: elapsed: %d.%09d[s] loop:%d\n",
			tid, i, (int)elapse.tv_sec, (int)elapse.tv_nsec, j);
		fflush(stdout);
	}

	return 0;
}
#endif /* _POSIX_TIMERS */

int test_time_poll(int tid)
{
	struct timeval st, ed, elapse;

	for (int i = 0; i < n_poll; i++) {
		int j = 0;
		gettimeofday(&st, NULL);
		timerclear(&elapse);

		while (elapse.tv_sec < 2) {
			gettimeofday(&ed, NULL);
			timersub(&ed, &st, &elapse);
			j++;
		}

		printf("tid %3d: %d: poll time : elapsed: %d.%06d[s] loop:%d\n",
			tid, i, (int)elapse.tv_sec, (int)elapse.tv_usec, j);
		fflush(stdout);
	}

	return 0;
}

void *thread_main(void *arg)
{
	int tid = (int)gettid();
	int r, ret = 0;

#ifdef _POSIX_TIMERS
	r = test_clock_poll(tid);
	if (r) {
		printf("tid %d: test_clock_poll failed.\n", tid);
		ret = r;
	}
#endif /* _POSIX_TIMERS */

	r = test_time_poll(tid);
	if (r) {
		printf("tid %d: test_time_poll failed.\n", tid);
		ret = r;
	}

	return (void *)(intptr_t)ret;
}

int test_main(int nt)
{
	void *val;
	int r, ret = 0;

	printf("----- test %d thread(s)\n", nt);
	fflush(stdout);

	for (int i = 0; i < nt - 1; i++) {
		r = pthread_create(&th[i], NULL, thread_main, NULL);
		if (r) {
			printf("%d: pthread_create: %s\n", i, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	if (nt > 0) {
		thread_main(NULL);
	}

	for (int i = 0; i < nt - 1; i++) {
		r = pthread_join(th[i], &val);
		if (r) {
			printf("%d: pthread_join: %s\n", i, strerror(r));
			ret = r;
		}
		fflush(stdout);
	}

	return ret;
}

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: test mt_clock start\n", argv[0]);

	for (int i = n_threads; i > 0; i--) {
		r = test_main(i);
		if (r) {
			printf("test %d thread(s) failed.\n", i);
			ret = r;
		}
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return ret;
}
