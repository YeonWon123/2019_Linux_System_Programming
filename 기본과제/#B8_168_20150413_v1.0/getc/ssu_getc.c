#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	int character;

	while ((character = getc(stdin)) != EOF)
		if (putc(character, stdout) == EOF) {
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
