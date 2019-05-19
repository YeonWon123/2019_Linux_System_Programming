#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);	

	char *fname = "ssu_test.txt";
	FILE *fp;
	long position;
	int character;

	if ((fp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", fname);
		exit(1);
	}

	printf("Input number >>");
	scanf("%ld", &position);
	fseek(fp, position - 1, SEEK_SET);
	character = getc(fp);
	printf("%ldth character => %c\n", position, character);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
