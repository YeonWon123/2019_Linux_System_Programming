#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(void)
{
	char buf[BUFFER_SIZE];
	int pid;
	int pipe_fd[2];

	if (pipe(pipe_fd) < 0) {
		fprintf(stderr, "pipe error\n");
		exit(1);
	}

	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if (pid == 0) {
		close(pipe_fd[0]);

		while(1) {
			memset(buf, 0x00, BUFFER_SIZE);
			sprintf(buf, "Hello Mother Process. My name is %d\n", getpid());
			write(pipe_fd[1], buf, strlen(buf));
			sleep(1);
		}
	}
	else {
		close(pipe_fd[1]);

		while (1) {
			memset(buf, 0x00, BUFFER_SIZE);
			read(pipe_fd[0], buf, BUFFER_SIZE);
			fprintf(stderr, "%s", buf);
		}
	}

	exit(0);
}
