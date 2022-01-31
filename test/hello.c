#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	printf("%s: hello world!\n", argv[0]);

	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %p\n", i, argv[i]);
	}

	return 0;
}
