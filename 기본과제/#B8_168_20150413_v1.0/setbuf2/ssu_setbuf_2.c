#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

#define BUFFER_SIZE 1024

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	char buf[BUFFER_SIZE];
	int a, b;
	int i;

	setbuf(stdin, buf);
	scanf("%d %d", &a, &b);

	for (i = 0; buf[i] != '\n'; i++)
		putchar(buf[i]);

	putchar('\n');

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
