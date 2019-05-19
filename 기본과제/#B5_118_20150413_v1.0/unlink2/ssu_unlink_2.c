#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
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

int main(void)
{
	struct timeval begin_t, end_t;
	char buf[64];
	char *fname = "ssu_tempfile";
	int fd;
	int length;

	gettimeofday(&begin_t, NULL);

	if ((fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0600)) < 0) {
		fprintf(stderr, "first open error for %s\n", fname);
		exit(1);
	}

	if (unlink(fname) < 0) {
		fprintf(stderr, "unlink error for %s\n", fname);
		exit(1);
	}

	if (write(fd, "How are you?", 12) != 12) {
		fprintf(stderr, "write error\n");
		exit(1);
	}

	lseek(fd, 0, 0);

	if ((length = read(fd, buf, sizeof(buf))) < 0) {
		fprintf(stderr, "buf read error\n");
		exit(1);
	}
	else
		buf[length] = 0;

	printf("%s\n", buf);
	close(fd);

	if((fd = open(fname, O_RDWR)) < 0) {
		fprintf(stderr, "second open error for %s\n", fname);
		exit(1);
	}
	else
		close(fd);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
