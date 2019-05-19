#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

#define BUFFER_SIZE 1024

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	char *fname = "ssu_test.dat";
	char name[BUFFER_SIZE];
	FILE *fp;
	int age;

	fp = fopen(fname, "r");
	fscanf(fp, "%s%d", name, &age);
	fclose(fp);
	fp = fopen(fname, "w");
	fprintf(fp, "%s is %d years old\n", name, age);
	fclose(fp);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
