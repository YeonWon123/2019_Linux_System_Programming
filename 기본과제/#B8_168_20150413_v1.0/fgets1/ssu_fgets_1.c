#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ssu_runtime.h"
#define BUFFER_SIZE 1024

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	char buf[BUFFER_SIZE];
	while(fgets(buf, BUFFER_SIZE, stdin) != NULL)
		if (fputs(buf, stdout) == EOF) {
			fprintf(stderr, "standard output error\n");
			exit(1);
		}

	if (ferror(stdin)) {
		fprintf(stderr, "standard input error\n");
		exit(1);
	}

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
