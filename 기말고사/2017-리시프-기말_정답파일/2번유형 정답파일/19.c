#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main(void) {
	char buf[BUFFER_SIZE];
	int pid;
	int pipe_fd[2];

	pipe(pipe_fd);

	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if (pid > 0) {
		printf(" CHILD: writing to the pipe \n");
		write(pipe_fd[1], "OSLAB", 6);
		printf(" CHILD: exiting\n");
		exit(0);
	}
	else {
		printf("PARENT: reading from pipe\n");
		read(pipe_fd[0], buf, 6);
		printf("PARENT: read \"%s \" \n", buf);
		wait(NULL);
	}

	exit(0);
}
