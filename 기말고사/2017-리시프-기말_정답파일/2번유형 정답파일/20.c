#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define SHORT_BUF 1024
#define MAX_BUF 4096
#define ASCII_MAX 256
#define ARG_MAX 16

char *arg_container[ASCII_MAX][ARG_MAX];
int arg_ptr[ASCII_MAX];

int ssu_proc_pid_fd(void);
int ssu_proc_pid_fd_derived(char *);

int main(void) 
{
	ssu_proc_pid_fd();
	exit(0);
}

int ssu_proc_pid_fd(void) {
	char fd_path[MAX_BUF] = {0};           // /proc/[PID]/fd의 경로를 저장하는 배열
	char pid_buf[SHORT_BUF] = {0};         // PID를 문자열으로 저장하는 배열
	char opt = 'f';                        // 옵션을 나타내는 문자
	int j = 0;

	pid_t pid = getpid();
	sprintf(pid_buf, "%d", pid);
	arg_container[opt][arg_ptr[opt]] = (char *)malloc(strlen(pid_buf) + 1);
	strncpy(arg_container[opt][arg_ptr[opt]++], pid_buf, strlen(pid_buf));

	for (j = 0; j < ARG_MAX; j++) {
		memset(fd_path,0,sizeof(fd_path));

		if (arg_container[opt][j] == NULL)		break;
		sprintf(fd_path,"/proc/%s/fd",arg_container[opt][j]);

		if (access(fd_path, F_OK) == 0) {	
			if (access(fd_path, R_OK) == 0) 	{
				if (ssu_proc_pid_fd_derived(fd_path) == -1)
					return -1;
			}
			else
				fprintf(stderr,"%s can't be read.\n", fd_path);
		}
		else
			fprintf(stderr,"%s doesn't exist.\n", fd_path);
	}

	return 0;
}

int ssu_proc_pid_fd_derived (char *fd_path) {
	char fd_res[SHORT_BUF] = {0};             // symbolic link를 readlink()로 읽은 결과를 저장하는 배열
	char fd_tmp[SHORT_BUF] = {0};          // 
	DIR *dir_ptr = NULL;
	struct dirent *dir_entry;

	if ((dir_ptr = opendir(fd_path)) == NULL) {
		fprintf(stderr, "opendir error for %s",fd_path);
		return -1;
	}

	while ((dir_entry = readdir(dir_ptr)) != NULL) {
		memset(fd_res,0,sizeof(fd_res));
		if ((strcmp(dir_entry -> d_name,".") == 0) 
				|| (strcmp(dir_entry -> d_name,"..") == 0))	continue;

		sprintf(fd_tmp,"%s/%s",fd_path,dir_entry->d_name);
		readlink(fd_tmp,fd_res,SHORT_BUF);
		printf("File Descriptor number: %s, Opened File: %s\n",dir_entry -> d_name,fd_res);
	}
	closedir(dir_ptr);
	return 0;
}
