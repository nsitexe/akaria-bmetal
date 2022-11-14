#include <stdio.h>
#include <sys/reboot.h>

int main(int argc, char *argv[], char *envp[])
{
	printf("power off!\n");
	fflush(stdout);

	reboot(RB_POWER_OFF);

	return 0;
}
