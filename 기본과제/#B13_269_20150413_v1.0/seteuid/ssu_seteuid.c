#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(void)
{
	int fd, state;

	state = seteuid(1000); // root권한이 아니면 실행 불가

	if (state < 0) {
		fprintf(stderr, "seteuid error\n");
		exit(1);
	}

	if ((fd = open("ssu_test.txt", O_CREAT | O_RDWR, S_IRWXU)) < 0) {
		fprintf(stderr, "open error\n");
		exit(1);
	}

	close(fd);
	exit(0);
}
