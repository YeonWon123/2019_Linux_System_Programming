#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "ssu_runtime.h"

#define DIRECTORY_SIZE MAXNAMLEN

int main(int argc, char *argv[])
{
	struct dirent *dentry;
	struct stat statbuf;
	char filename[DIRECTORY_SIZE+1];
	DIR *dirp;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <directory>\n", argv[0]);
		exit(1);
	}

	if ((dirp = opendir(argv[1])) == NULL || chdir(argv[1]) == -1) {
		fprintf(stderr, "opendir, chdir error for %s\n", argv[1]);
		exit(1);
	}

	while ((dentry = readdir(dirp)) != NULL) {
		if (dentry->d_ino == 0)
			continue;

		memcpy(filename, dentry->d_name, DIRECTORY_SIZE);

		if (stat(filename, &statbuf) == -1) {
			fprintf(stderr, "stat error for %s\n", filename);
			break;
		}

		if ((statbuf.st_mode & S_IFMT) == S_IFREG)
			printf("%-14s %ld\n", filename, statbuf.st_size);
		else
			printf("%-14s\n", filename);
	}

	exit(0);
}
