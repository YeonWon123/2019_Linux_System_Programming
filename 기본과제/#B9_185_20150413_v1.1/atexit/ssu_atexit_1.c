#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

void ssu_out(void);

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	if (atexit(ssu_out)) {
		fprintf(stderr, "atexit error\n");
		exit(1);
	}

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}

void ssu_out(void) {
	printf("atexit succeeded!\n");
}
