#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

#define BUFFER_SIZE 1024

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	char buf[BUFFER_SIZE];
	char name[L_tmpnam];
	FILE *fp;

	printf("Temporary filename <<%s>>\n", tmpnam(name));

	if ((fp = tmpfile()) == NULL) {
		fprintf(stderr, "tmpfile create error!!\n");
		exit(1);
	}

	fputs("create tmpfile success!!\n", fp);
	rewind(fp);
	fgets(buf, sizeof(buf), fp);
	puts(buf);
	
	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
