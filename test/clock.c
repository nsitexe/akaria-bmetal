#include <ctype.h>
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

#ifdef _POSIX_TIMERS
int test_clock_gettime(void)
{
	struct timespec st, ed, elapse;
	int r;

	r = clock_gettime(CLOCK_REALTIME, &st);
	if (r) {
		printf("clock_gettime(REALTIME, start) is failed.\n");
		return -1;
	}

	r = clock_gettime(CLOCK_REALTIME, &ed);
	if (r) {
		printf("clock_gettime(REALTIME, end) is failed.\n");
		return -1;
	}

	timespecsub(&ed, &st, &elapse);
	printf("clock_gettime: real: start  : %lld.%09d[s]\n",
		(long long)st.tv_sec, (int)st.tv_nsec);
	printf("clock_gettime: real: elapsed: %d.%09d[s]\n",
		(int)elapse.tv_sec, (int)elapse.tv_nsec);

	r = clock_gettime(CLOCK_MONOTONIC, &st);
	if (r) {
		printf("clock_gettime(MONOTONIC, start) is failed.\n");
		return -1;
	}

	r = clock_gettime(CLOCK_MONOTONIC, &ed);
	if (r) {
		printf("clock_gettime(MONOTONIC, end) is failed.\n");
		return -1;
	}

	timespecsub(&ed, &st, &elapse);
	printf("clock_gettime: mono: start  : %lld.%09d[s]\n",
		(long long)st.tv_sec, (int)st.tv_nsec);
	printf("clock_gettime: mono: elapsed: %d.%09d[s]\n",
		(int)elapse.tv_sec, (int)elapse.tv_nsec);

	return 0;
}

int test_clock_settime(void)
{
	struct timespec ts_before, ts, ts_after;
	int r;

	r = clock_gettime(CLOCK_REALTIME, &ts_before);
	if (r) {
		printf("clock_gettime(REALTIME, before) is failed.\n");
		return -1;
	}

	ts.tv_sec = 10000000000ULL;
	ts.tv_nsec = 123456789;
	r = clock_settime(CLOCK_REALTIME, &ts);
	if (r) {
		printf("clock_settime(REALTIME) is failed.\n");
		return -1;
	}

	r = clock_gettime(CLOCK_REALTIME, &ts_after);
	if (r) {
		printf("clock_gettime(REALTIME, after) is failed.\n");
		return -1;
	}

	printf("clock_settime: before: %lld.%09d[s]\n",
		(long long)ts_before.tv_sec, (int)ts_before.tv_nsec);
	printf("clock_settime: set   : %lld.%09d[s]\n",
		(long long)ts.tv_sec, (int)ts.tv_nsec);
	printf("clock_settime: after : %lld.%09d[s]\n",
		(long long)ts_after.tv_sec, (int)ts_after.tv_nsec);

	/* MONOTONIC clock is read only */
	r = clock_settime(CLOCK_MONOTONIC, &ts);
	if (!r) {
		printf("clock_settime(MONOTONIC) is read only.\n");
		return -1;
	}

	return 0;
}

int test_clock_poll_5(void)
{
	struct timespec st, ed, elapse;

	for (int i = 0; i < 5; i++) {
		int j = 0;
		clock_gettime(CLOCK_REALTIME, &st);
		timespecclear(&elapse);

		while (elapse.tv_sec < 2) {
			clock_gettime(CLOCK_REALTIME, &ed);
			timespecsub(&ed, &st, &elapse);
			j++;
		}

		printf("%d: poll clock: elapsed: %d.%09d[s] loop:%d\n",
			i, (int)elapse.tv_sec, (int)elapse.tv_nsec, j);
		fflush(stdout);
	}

	return 0;
}
#endif /* _POSIX_TIMERS */

int test_gettimeofday(void)
{
	struct timeval st, ed, elapse;
	int r;

	r = gettimeofday(&st, NULL);
	if (r) {
		printf("gettimeofday(REALTIME, start) is failed.\n");
		return -1;
	}

	r = gettimeofday(&ed, NULL);
	if (r) {
		printf("gettimeofday(REALTIME, end) is failed.\n");
		return -1;
	}

	timersub(&ed, &st, &elapse);
	printf("gettimeofday: start  : %lld.%06d[s]\n",
		(long long)st.tv_sec, (int)st.tv_usec);
	printf("gettimeofday: elapsed: %d.%06d[s]\n",
		(int)elapse.tv_sec, (int)elapse.tv_usec);

	return 0;
}

int test_time_poll_5(void)
{
	struct timeval st, ed, elapse;

	for (int i = 0; i < 5; i++) {
		int j = 0;
		gettimeofday(&st, NULL);
		timerclear(&elapse);

		while (elapse.tv_sec < 2) {
			gettimeofday(&ed, NULL);
			timersub(&ed, &st, &elapse);
			j++;
		}

		printf("%d: poll time : elapsed: %d.%06d[s] loop:%d\n",
			i, (int)elapse.tv_sec, (int)elapse.tv_usec, j);
		fflush(stdout);
	}

	return 0;
}

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: test clock start\n", argv[0]);

#ifdef _POSIX_TIMERS
	r = test_clock_gettime();
	if (r) {
		printf("%s: test_clock_gettime failed.\n", argv[0]);
		ret = r;
	}

	r = test_clock_settime();
	if (r) {
		printf("%s: test_clock_settime failed.\n", argv[0]);
		ret = r;
	}

	r = test_clock_poll_5();
	if (r) {
		printf("%s: test_clock_poll_5 failed.\n", argv[0]);
		ret = r;
	}
#endif /* _POSIX_TIMERS */

	r = test_gettimeofday();
	if (r) {
		printf("%s: test_gettimeofday failed.\n", argv[0]);
		ret = r;
	}

	r = test_time_poll_5();
	if (r) {
		printf("%s: test_time_poll_5 failed.\n", argv[0]);
		ret = r;
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return ret;
}
