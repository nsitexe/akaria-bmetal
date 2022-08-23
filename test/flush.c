#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	int r, ret = 0;

	printf("%s: test flush start\n", argv[0]);
	r = fflush(stdout);
	if (r) {
		printf("%s: fflush(stdout) failed.\n", argv[0]);
		fflush(NULL);
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
