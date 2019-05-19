#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define SECOND_TO_MICRO 1000000
#define S_MODE 0644
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

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;
	char buf[BUFFER_SIZE];
	int fd1, fd2;
	int length;

	gettimeofday(&begin_t, NULL);

	if (argc != 3) {
		fprintf(stderr, "Usage : %s filein fileout\n", argv[0]);
		exit(1);
	}

	if ((fd1 = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	if ((fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_MODE)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[2]);
		exit(1);
	}

	while ((length = read(fd1, buf, BUFFER_SIZE)) > 0)
		write(fd2, buf, length);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
