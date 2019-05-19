#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "ssu_employee.h"

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
	struct timeval begin_t, end_t;
	struct ssu_employee record;
	int fd;

	gettimeofday(&begin_t, NULL);

	if (argc < 2) {
		fprintf(stderr, "usage : %s file\n", argv[0]);
		exit(1);
	}

	if ((fd = open(argv[1], O_WRONLY | O_CREAT | O_EXCL, 0640)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	while (1) {
		printf("Enter employee name <SPACE> salary: ");
		scanf("%s", record.name);

		if (record.name[0] == '.')
			break;

		scanf("%d", &record.salary);
		record.pid = getpid();
		write(fd, (char *)&record, sizeof(record));
	}

	close(fd);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
