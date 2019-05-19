#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	char *fname = "ssu_test.txt";
	int fd;

	printf("First printf : Hello, OSLAB!!\n");

	if ((fd = open(fname, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

	if (freopen(fname, "w", stdout) != NULL)
		printf("Second printf : Hello, OSLAB!!\n");

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
