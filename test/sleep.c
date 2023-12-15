#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

int test_usleep(void)
{
	struct timeval st, ed, elapse;
	int r;

	r = gettimeofday(&st, NULL);
	if (r) {
		perror("gettimeofday(usleep, start)");
		return -1;
	}

	r = usleep(1000);
	if (r) {
		perror("usleep()");
		return -1;
	}

	r = gettimeofday(&ed, NULL);
	if (r) {
		perror("gettimeofday(usleep, end)");
		return -1;
	}

	timersub(&ed, &st, &elapse);
	printf("test_usleep: start  : %lld.%06d[s]\n",
		(long long)st.tv_sec, (int)st.tv_usec);
	printf("test_usleep: elapsed: %d.%06d[s]\n",
		(int)elapse.tv_sec, (int)elapse.tv_usec);

	return 0;
}

int test_nanosleep(void)
{
	struct timeval st, ed, elapse;
	struct timespec ts;
	int r;

	r = gettimeofday(&st, NULL);
	if (r) {
		perror("gettimeofday(nanosleep, start)");
		return -1;
	}

	ts.tv_sec = 0;
	ts.tv_nsec = 1000000;
	r = nanosleep(&ts, &ts);
	if (r) {
		perror("nanosleep()");
		return -1;
	}

	r = gettimeofday(&ed, NULL);
	if (r) {
		perror("gettimeofday(nanosleep, end)");
		return -1;
	}

	timersub(&ed, &st, &elapse);
	printf("test_nanosleep: start  : %lld.%06d[s]\n",
		(long long)st.tv_sec, (int)st.tv_usec);
	printf("test_nanosleep: elapsed: %d.%06d[s]\n",
		(int)elapse.tv_sec, (int)elapse.tv_usec);

	return 0;
}

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: test sleep start\n", argv[0]);

	r = test_usleep();
	if (r) {
		ret = r;
	}

	r = test_nanosleep();
	if (r) {
		ret = r;
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}
