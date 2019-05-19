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

	pid_t child1, child2;
	int pid, status;

	if ((child1 = fork()) == 0)
		execlp("date", "date", (char *)0);

	if ((child2 = fork()) == 0)
		execlp("who", "who", (char *)0);

	printf("parent: waiting for children\n");

	while ((pid = wait(&status)) != -1) {
		if (child1 == pid)
			printf("parent: first child: %d\n", (status >> 8));
		else if (child2 == pid)
			printf("parent; second child: %d\n", (status >> 8));
	}

	printf("parent: all children terminated\n");
	
	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
