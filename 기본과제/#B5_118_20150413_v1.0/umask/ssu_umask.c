#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#define RW_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
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


int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);
	
	char *fname1 = "ssu_file1";
	char *fname2 = "ssu_file2";

	umask(0);

	if (creat(fname1, RW_MODE) < 0) {
		fprintf(stderr, "creat error for %s\n", fname1);
		exit(1);
	}

	umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	if (creat(fname2, RW_MODE) < 0) {
		fprintf(stderr, "creat error for %s\n", fname2);
		exit(1);
	}

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
