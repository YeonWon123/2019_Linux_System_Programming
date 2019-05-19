#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define SECOND_TO_MICRO 1000000
#define TABLE_SIZE 128
#define BUFFER_SIZE 1024

void ssu_runtime(struct timeval* begin_t, struct timeval* end_t) {
	end_t -> tv_sec -= begin_t -> tv_sec;

	if(end_t -> tv_usec < begin_t -> tv_usec) {
		end_t -> tv_sec--;
		end_t -> tv_usec += SECOND_TO_MICRO;
	}

	end_t -> tv_usec -= begin_t -> tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t -> tv_sec, end_t -> tv_usec);
}

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;
	static struct {
		long offset;
		int length;
	} table [TABLE_SIZE];
	char buf[BUFFER_SIZE];
	long offset;
	int entry;
	int i;
	int length;
	int fd;

	gettimeofday(&begin_t, NULL);

	if (argc < 2) {
		fprintf(stderr, "usage: %s <file>\n", argv[0]);
		exit(1);
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	entry = 0;
	offset = 0;
	while ((length = read(fd, buf, BUFFER_SIZE)) > 0) {
		for(i = 0 ; i < length ; i++) {
			table[entry].length++;
			offset++;

			if (buf[i] == '\n')
				table[++entry].offset = offset;
		}
	}

#ifdef DEBUG
	for(i = 0 ; i < entry ; i++)
		printf("%d : %;d, %d\n", i + 1, table[i].offset, table[i].length);
#endif

	while (1) {
		printf("Enter line number : ");
		scanf("%d", &length);

		if (--length < 0)
			break;

		lseek(fd, table[length].offset, 0);

		if(read(fd, buf, table[length].length) <= 0)
			continue;

		buf[table[length].length] = '\0';
		printf("%s", buf);
	}

	close(fd);

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
