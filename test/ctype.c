#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

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

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: hello ctype!\n", argv[0]);
	fflush(stdout);

	r = test_ctype();
	if (r) {
		printf("%s: test_ctype failed.\n", argv[0]);
		ret = r;
	}
	fflush(stdout);

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}
	fflush(stdout);

	return ret;
}
