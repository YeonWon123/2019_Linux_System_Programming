#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *ssu_thread(void *arg);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int glo_val = 0;

int main(void) {
	pthread_t tid1, tid2;
	pthread_create(&tid1, NULL, &ssu_thread, (void *) 1);
	pthread_create(&tid2, NULL, &ssu_thread, (void *) 2);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	
	printf("final value : %d\n", glo_val);
	exit(0);
}

void *ssu_thread(void *arg) {
	int num = (int) arg;
	while (glo_val < 10) {
		pthread_mutex_lock(&lock);
		if(glo_val>=10)
			break;
		glo_val++;
		printf("global value ssu_thread%d: %d\n", num, glo_val);
		pthread_mutex_unlock(&lock);
	}
	pthread_mutex_unlock(&lock);
}
