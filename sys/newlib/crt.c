/* SPDX-License-Identifier: Apache-2.0 */

void __libc_init_array(void);
void __libc_fini_array(void);
int atexit(void (*function)(void));
void exit(int status);
int main(int argc, char *argv[], char *envp[]);

int errno_real = 0;

int *__errno(void)
{
	return &errno_real;
}

void __libc_init(int argc, char *argv[], char *envp[])
{
	atexit(__libc_fini_array);
	__libc_init_array();

	int r = main(argc, argv, envp);

	exit(r);
}
