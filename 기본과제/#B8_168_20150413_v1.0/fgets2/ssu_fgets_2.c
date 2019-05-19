#include <stdio.h>
#include <stdlib.h>
#include "ssu_runtime.h"

#define BUFFER_MAX 256

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);	

	char command[BUFFER_MAX];
	char *prompt = "Prompt>>";

	while (1) {
		fputs(prompt, stdout);

		if (fgets(command, sizeof(command), stdin) == NULL)
			break;

		system(command);
	}

	fprintf(stdout, "Good bye...\n");
	fflush(stdout);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
