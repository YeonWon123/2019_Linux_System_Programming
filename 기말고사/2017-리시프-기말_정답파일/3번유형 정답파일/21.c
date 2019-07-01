#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

char * buf;
int readsize;

void *read_thread(void *arg) {
	int num = (int)arg;
	int fd = open("test.txt", O_RDONLY);
	lseek(fd, readsize * num, SEEK_SET);
	read(fd, &buf[readsize * num], readsize);
	close(fd);
}

int main(void) {	
	struct stat stat_buf;
	int	status;
	int thread_num;
	pthread_t *tid;
	
	printf("Number of thread : ");
	scanf("%d", &thread_num);
	tid = (pthread_t *) malloc(thread_num * sizeof(pthread_t));

	status = stat("test.txt", &stat_buf);
	if(status == 0) {
		readsize = (stat_buf.st_size)/thread_num;
		buf = (char*) malloc(stat_buf.st_size);
	}
	for(int i = 0; i < thread_num; i++)
		pthread_create(&tid[i], NULL, read_thread, (void *)i);
	for(int i = 0; i < thread_num; i++)
		pthread_join(tid[i], NULL);
	
	printf("%s\n",buf);
	free(buf);
	free(tid);
}
