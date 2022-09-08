#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef timespecsub
#define timespecsub(a, b, c)                                \
	do {                                                \
		(c)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
		(c)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
		if ((c)->tv_nsec < 0) {                     \
			(c)->tv_sec -= 1;                   \
			(c)->tv_nsec += 1000000000L;        \
		}                                           \
	} while (0)
#endif

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
	printf("clock_gettime: elapsed: %d.%09d[s]\n", (int)elapse.tv_sec, (int)elapse.tv_nsec);

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
	printf("gettimeofday: elapsed: %d.%06d[s]\n", (int)elapse.tv_sec, (int)elapse.tv_usec);

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
#endif /* _POSIX_TIMERS */

	r = test_gettimeofday();
	if (r) {
		printf("%s: test_gettimeofday failed.\n", argv[0]);
		ret = r;
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return ret;
}
