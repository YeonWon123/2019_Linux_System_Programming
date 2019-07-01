#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *ssu_thread1(void *arg);
void *ssu_thread2(void *arg);

pthread_t glo_tid;

int main(void)
{
    pthread_t loc_tid[2];
    int i;

    if (pthread_create(&loc_tid[0], NULL, ssu_thread1, NULL) != 0) {
	fprintf(stderr, "pthread_create error\n");
	exit(1);
    }

    if (pthread_create(&loc_tid[1], NULL, ssu_thread2, NULL) != 0) {
	fprintf(stderr, "pthread_create error\n");
	exit(1);
    }

    sleep(5);

    for (i = 0; i < 2; i++) {
	if (pthread_equal(loc_tid[i], glo_tid) != 0) {
	    if(i == 0)
		printf("first ");
	    else 
		printf("second ");
	    printf("thread assigns it's tid to global tid\n");
	}
    }
    exit(0);
}

void *ssu_thread1(void *arg) {
    printf("in ssu_thread1\n");
    return NULL;
}

void *ssu_thread2(void *arg) {
    printf("in ssu_thread2\n");
    glo_tid = pthread_self();
    return NULL;
}
