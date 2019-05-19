#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define TABLE_SIZE (sizeof(table)/sizeof(*table))
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
	struct {
		char *text;
		int mode;
	} table[] = {
		{"exists", 0},
		{"execute", 1},
		{"write", 2},
		{"read", 4}
	};
	int i;
	struct timeval begin_t, end_t;

	gettimeofday(&begin_t, NULL);

	if (argc < 2) {
		fprintf(stderr, "usage : %s <file>\n", argv[0]);
		exit(1);
	}

	for (i = 0; i < TABLE_SIZE; i++) {
		if (access(argv[1], table[i].mode) != -1)
			printf("%s -ok\n", table[i].text);
		else
			printf("%s\n", table[i].text);
	}


	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
