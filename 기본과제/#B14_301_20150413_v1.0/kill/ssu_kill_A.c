#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s [Process ID]\n", argv[0]);
		exit(1);
	}
	else
		kill(atoi(argv[1]), SIGKILL);

	exit(0);
}
