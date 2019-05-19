#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

#define MAX_LINE 4096

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	char buf[MAX_LINE];
	char name[L_tmpnam];
	FILE *fp;

	printf("temp file 1 : %s\n", tmpnam(NULL));
	tmpnam(name);
	printf("temp file 2 : %s\n", name);

	if ((fp = tmpfile()) == NULL) {
		fprintf(stderr, "tmpfile error\n");
		exit(1);
	}

	fputs("tmpfile created temporary file.\n", fp);
	fseek(fp, 0, SEEK_SET);

	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fprintf(stderr, "fgets error\n");
		exit(1);
	}

	fputs(buf, stdout);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
