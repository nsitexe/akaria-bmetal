#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int test_malloc1(int size)
{
	char *buf;

	printf("malloc(%d)\n", size);

	buf = malloc(size);
	if (!buf) {
		printf("malloc(%d) failed.\n", size);
		return -1;
	}
	printf("  buf   (%p)\n", buf);

	printf("  memset(%p)\n", buf);
	memset(buf, 0, size);

	printf("  free  (%p)\n", buf);
	free(buf);

	return 0;
}

int test_malloc2(int size1, int size2)
{
	char *buf1 = NULL, *buf2 = NULL;
	int ret = 0;

	printf("malloc(%d, %d)\n", size1, size2);

	buf1 = malloc(size1);
	if (!buf1) {
		printf("malloc 1(%d) failed.\n", size1);
		ret = -1;
		goto err_out;
	}
	printf("  buf 1   (%p)\n", buf1);

	buf2 = malloc(size2);
	if (!buf2) {
		printf("malloc 2(%d) failed.\n", size2);
		ret = -2;
		goto err_out;
	}
	printf("  buf 2   (%p)\n", buf2);

	printf("  memset 1(%p)\n", buf1);
	memset(buf1, 0, size1);

	printf("  memset 2(%p)\n", buf2);
	memset(buf2, 1, size2);

err_out:
	printf("  free 1  (%p)\n", buf1);
	free(buf1);

	printf("  free 2  (%p)\n", buf2);
	free(buf2);

	return ret;
}

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: malloc\n", argv[0]);
	fflush(stdout);

	r = test_malloc1(32);
	if (r) {
		ret = r;
	}
	r = test_malloc1(10000);
	if (r) {
		ret = r;
	}
	r = test_malloc1(100000);
	if (r) {
		ret = r;
	}

	r = test_malloc2(1000, 100000);
	if (r) {
		ret = r;
	}
	r = test_malloc2(10000, 1000000);
	if (r) {
		ret = r;
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}
	fflush(stdout);

	return ret;
}
