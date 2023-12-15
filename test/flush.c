#include <stdio.h>

int test_fflush_simple(void)
{
	int r, ret = 0;

	r = fflush(stdout);
	if (r) {
		perror("fflush(stdout)");
		ret = r;
	}

	r = fflush(stderr);
	if (r) {
		perror("fflush(stderr)");
		ret = r;
	}

	r = fflush(NULL);
	if (r) {
		perror("fflush(NULL)");
		ret = r;
	}

	return ret;
}

int test_fflush_print(void)
{
	int r, ret = 0;

	printf("fprintf with fflush()\n");
	fflush(NULL);

	for (int i = 0; i < 5; i++) {
		r = fprintf(stdout, "  to stdout %d.\n", i);
		if (r < 0) {
			ret = r;
		}

		r = fprintf(stderr, "  to stderr %d.\n", i);
		if (r < 0) {
			ret = r;
		}
		fflush(NULL);
	}
	fflush(NULL);

	printf("fprintf without fflush()\n");
	fflush(NULL);

	for (int i = 0; i < 5; i++) {
		r = fprintf(stdout, "  to stdout %d.\n", i);
		if (r < 0) {
			ret = r;
		}

		r = fprintf(stderr, "  to stderr %d.\n", i);
		if (r < 0) {
			ret = r;
		}
	}
	fflush(NULL);

	return ret;
}

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: test flush start\n", argv[0]);

	r = test_fflush_simple();
	if (r) {
		printf("%s: test_fflush_simple() failed.\n", argv[0]);
		ret = r;
	}

	r = test_fflush_print();
	if (r) {
		printf("%s: test_fflush_print() failed.\n", argv[0]);
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
