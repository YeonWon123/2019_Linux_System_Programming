#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define SECOND_TO_MICRO 1000000
#define BUFFER_SIZE 1024

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
	char buf[BUFFER_SIZE];
	char *fname = "ssu_test.txt";
	int count;
	int fd1, fd2;

	gettimeofday(&begin_t, NULL);

	fd1 = open(fname, O_RDONLY, 0644);
	fd2 = open(fname, O_RDONLY, 0644);

	if(fd1 < 0 || fd2 < 0) {
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

	count = read(fd1, buf, 25);
	buf[count] = 0;
	printf("fd1's first printf : %s\n", buf);
	lseek(fd1, 1, SEEK_CUR);
	count = read(fd1, buf, 24);
	buf[count] = 0;
	printf("fd1's second printf : %s\n", buf);
	count = read(fd2, buf, 25);
	buf[count] = 0;
	printf("fd2's first printf : %s\n", buf);
	lseek(fd2, 1, SEEK_CUR);
	count = read(fd2, buf, 24);
	buf[count] = 0;
	printf("fd2's second printf : %s\n", buf);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
