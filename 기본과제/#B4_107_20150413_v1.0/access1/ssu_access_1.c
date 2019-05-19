#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define SECOND_TO_MICRO 1000000

void ssu_runtime(struct timeval* begin_t, struct timeval* end_t) {
	end_t -> tv_sec -= begin_t -> tv_sec;

	if (end_t -> tv_usec < begin_t -> tv_usec) {
		end_t -> tv_sec--;
		end_t -> tv_usec += SECOND_TO_MICRO;
	}
	
	end_t -> tv_usec -= begin_t -> tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t -> tv_sec, end_t -> tv_usec);
}

int main(int argc, char *argv[])
{
	int i;
	struct timeval begin_t, end_t;

	gettimeofday(&begin_t, NULL);

	if (argc < 2) {
		fprintf(stderr, "usage: %s <file1> <file2> .. <fileN>\n", argv[0]);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		if (access(argv[i], F_OK) < 0) {
			fprintf(stderr, "%s doesn't exist.\n", argv[i]);
			continue;
		}

		if (access(argv[i], R_OK) == 0)
			printf("User can read %s\n", argv[i]);

		if (access(argv[i], W_OK) == 0)
			printf("User can write %s\n", argv[i]);

		if (access(argv[i], X_OK) == 0)
			printf("User can execute %s\n", argv[i]);
					
	}

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
