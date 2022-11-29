#include <stdio.h>

#define EOT    '\4'

int main(int argc, char *argv[], char *envp[])
{
	int c = 0;

	printf("hello echo! (press Ctrl+D to exit)\n----\n");

	while (1) {
		fflush(stdout);

		c = getchar();
		if (c == EOF || c == EOT) {
			break;
		}

		putchar(c);

		if (c == '\r') {
			putchar('\n');
		}
	}

	printf("\n----\ngood bye echo!\n");

	return 0;
}
