#include <stdio.h>

#include <bmetal/bmetal.h>
#include <bmetal/syscall.h>

int main(int argc, char *argv[], char *envp[])
{
	printf("hello world!\n");

	printf("value of argv[%d]: %d\n", 4, *((int *)argv[4]));

	printf("argc:%d\n", argc);
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %p\n", i, argv[i]);
	}

	return 0;
}
