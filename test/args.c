#include <stdio.h>

int test_args(int argc, char *argv[])
{
	if (argc <= 0) {
		printf("argc %d is zero or negative.\n", argc);
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
	int r, ret = 0;

	printf("%s: test args start\n", argv[0]);

	r = test_args(argc, argv);
	if (r) {
		printf("test_args failed.\n");
		ret = r;
	}

	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %p\n", i, argv[i]);
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return ret;
}
