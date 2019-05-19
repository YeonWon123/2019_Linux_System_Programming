#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

int main(void) {
	struct timeval begin_t, end_t;
	char *fname = "ssu_test.txt";
	FILE *fp;

	gettimeofday(&begin_t, NULL);

	if ((fp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}
	else {
		printf("Success!\n");
		printf("Opening \"s\" in \"r\" mode!\n", fname);
	}

	fclose(fp);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
