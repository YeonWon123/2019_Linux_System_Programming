#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
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
	struct stat file_info;
	char *str;
	int i;
	struct timeval begin_t, end_t;

	gettimeofday(&begin_t, NULL);

	for (i = 1; i < argc; i++) {
		printf("name = %s, type = ", argv[i]);

		if (lstat(argv[i], &file_info) < 0) {
			fprintf(stderr, "lstat error\n");
			continue;
		}

		if (S_ISREG(file_info.st_mode))
			str = "regular";
		else if (S_ISDIR(file_info.st_mode))
			str = "directory";
		else if (S_ISCHR(file_info.st_mode))
			str = "character special";
		else if (S_ISBLK(file_info.st_mode))
			str = "block special";
		else if (S_ISFIFO(file_info.st_mode))
			str = "FIFO";
		else if (S_ISLNK(file_info.st_mode))
			str = "symbolic link";
		else if (S_ISSOCK(file_info.st_mode))
			str = "socket";
		else
			str = "unknown mode";

		printf("%s\n", str);
	}

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
