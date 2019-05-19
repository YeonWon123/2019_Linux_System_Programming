#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "ssu_runtime.h"

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	printf("Process ID		= %d\n", getpid());
	printf("Parent process ID 	= %d\n", getppid());
	printf("Real user ID 		= %d\n", getuid());
	printf("Effective user ID 	= %d\n", geteuid());
	printf("Real group ID 		= %d\n", getgid());
	printf("Effectiver group ID 	= %d\n", getegid());

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
