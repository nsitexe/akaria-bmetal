#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

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

int main(int argc, char *argv[], char *envp[])
{
	printf("%s: hello world!\n", argv[0]);

	printf("pid:%d\n", getpid());
	fflush(stdout);

	if (test_args(argc, argv)) {
		printf("test_args failed.\n");
	}
	fflush(stdout);

	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %p\n", i, argv[i]);
	}
	fflush(stdout);

	return 0;
}
