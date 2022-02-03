#include <stdint.h>
#include <stdio.h>

void vecadd(const double *a, const double *b, double *c, int n)
{
	for (int i = 0; i < n; i++) {
		c[i] = a[i] + b[i];

		if (i < 10) {
			printf("%d: a(%f) + b(%f) = c(%f)\n", i, a[i], b[i], c[i]);
		}
	}
}

int main(int argc, char *argv[], char *envp[])
{
	printf("%s: vecadd start\n", argv[0]);

	printf("argc: %d\n", argc);
	if (argc > 4) {
		double *a = (double *)argv[1];
		double *b = (double *)argv[2];
		double *c = (double *)argv[3];
		int *n = (int *)argv[4];

		printf("vector length: %d\n", *n);
		vecadd(a, b, c, *n);
	}

	return 0;
}
