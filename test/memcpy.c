#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

/* Length of test block, number of test blocks. */
#define TEST_BLOCK_LEN 4096
#define TEST_BLOCK_COUNT 8000

static uint8_t g_block_in[TEST_BLOCK_LEN];
static uint8_t g_block_out[TEST_BLOCK_LEN];

int bench(void)
{
	struct timeval end, start, elapse;
	uint8_t *block_in = g_block_in;
	uint8_t *block_out = g_block_out;
	int r = 0;
	size_t i;
	long long bytes, mills;

	printf("memcpy: %d x %d-byte blocks\n",
		TEST_BLOCK_LEN, TEST_BLOCK_COUNT);
	fflush(stdout);

	for (i = 0; i < TEST_BLOCK_LEN; i++) {
		block_in[i] = (uint8_t)(i & 0xff);
	}

	gettimeofday(&start, NULL);

	for (i = 0; i < TEST_BLOCK_COUNT; i++) {
		memcpy(block_out, block_in, TEST_BLOCK_LEN);
	}

	gettimeofday(&end, NULL);
	timersub(&end, &start, &elapse);

	for (i = 0; i < TEST_BLOCK_LEN; i++) {
		if (block_out[i] != (uint8_t)(i & 0xff)) {
			r = 1;
		}
	}

	bytes = (long long)TEST_BLOCK_LEN * (long long)TEST_BLOCK_COUNT;
	mills = (long long)elapse.tv_sec * 1000 + elapse.tv_usec / 1000;

	printf("time: %d.%03d [s]\n",
		(int)(mills / 1000), (int)(mills % 1000));
	printf("speed: %d bytes/s\n",
		(int)(bytes * 1000 / mills));

	return r;
}

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: bench memcpy start\n", argv[0]);

	r = bench();
	if (r) {
		printf("%s: bench_memcpy failed.\n", argv[0]);
		ret = r;
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}
