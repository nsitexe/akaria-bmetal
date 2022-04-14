#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define NOP    __asm volatile("nop")

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

	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %p\n", i, argv[i]);
	}

	return 0;
}
