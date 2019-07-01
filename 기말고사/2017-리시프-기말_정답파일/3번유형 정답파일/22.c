#include <stdio.h>
#include <pthread.h>

void *ssu_thread(void *arg);
void add_sum(int);
void finish_thread(void);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int sum;
int finish;


int main(void)
{
	pthread_t tid;
	int loc_sum = 0;
	int i;

	for(i=11; i < 100; i+=10){
		pthread_create(&tid, NULL, ssu_thread, (void*)i);
	}

	printf("start : 1, main thread\n");

	for(i=1; i <= 10; i++){
		loc_sum += i;
	}

	add_sum(loc_sum);
	finish_thread();
}

void *ssu_thread(void *arg){
	int loc_sum = 0;
	int start;
	int i;

	start = (int)arg;
	printf("start : %d\n", start);

	for(i = 0; i <= 9; i++){
		loc_sum += start + i;
	}

	add_sum(loc_sum);
	finish_thread();

}

void add_sum(int loc_sum){
	pthread_mutex_lock(&mutex);
	sum += loc_sum;
	pthread_mutex_unlock(&mutex);
}

void finish_thread(void){
	pthread_mutex_lock(&mutex);
	finish++;
	if(finish == 10)
		printf("total : %d\n", sum);
	pthread_mutex_unlock(&mutex);
	pthread_exit((void*)0);
}
