#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main(int argc, char *argv[])
{
	int fdin, fdout;
	void *src, *dst;
	struct stat statbuf;

	if (argc != 3) {
		printf("usage: %s <fromfile> <tofile>", argv[0]);
		exit(1);
	}

	if ((fdin = open(argv[1], O_RDONLY)) < 0) {
		printf("can't open %s for reading", argv[1]);
		exit(1);
	}

	if ((fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) < 0) {
		printf("cannot creat %s for writing", argv[2]);
		exit(1);
	}

	if (fstat(fdin, &statbuf) < 0) { /* 입력 파일의 크기 */
		printf("fstat() error");
		exit(1);
	}
	
	/* 출력 파일의 크기 */
	if (lseek(fdout, statbuf.st_size - 1, SEEK_SET) == -1) {
		printf("lseek() error");
		exit(1);
	}

	if (write(fdout, "", 1) != 1) {
		printf("write() error");
		exit(1);
	}

	if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED) {
		printf("mmap() error for input");
		exit(1);
	}

	if ((dst = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == MAP_FAILED) {
		printf("mmap() error for output");
		exit(1);
	}

	memcpy(dst, src, statbuf.st_size); /* 파일 복사를 수행 */
	exit(0);
}
