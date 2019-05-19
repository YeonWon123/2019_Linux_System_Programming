#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ssu_runtime.h"

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	if (fork() == 0)
		execl("/bin/echo", "echo", "this is", "message one", (char *)0);

	if (fork() == 0)
		execl("/bin/echo", "echo", "this is", "message two", (char *)0);

	printf("parent: waiting for children\n");

	while (wait((int *)0) != -1);

	printf("parent: all children terminated\n");

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
