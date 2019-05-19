#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	if (fork() == 0) {
		execl("/bin/echo", "echo", "this is", "message one", (char *)0);
		fprintf(stderr, "exec error\n");
		exit(1);
	}

	if (fork() == 0) {
		execl("/bin/echo", "echo", "this is", "message two", (char *)0);
		fprintf(stderr, "exec error\n");
		exit(1);
	}

	if (fork() == 0) {
		execl("/bin/echo", "echo", "this is", "message three", (char *)0);
		fprintf(stderr, "exec error\n");
		exit(1);
	}

	printf("Parent program ending\n");
	exit(0);
}
