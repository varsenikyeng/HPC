#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define M 6

pthread_barrier_t barrier1;
pthread_barrier_t barrier2;
pthread_barrier_t barrier3;

void* work(void* arg){
	int id = *(int*)arg;
	int time1 = rand()%3 + 1;
	sleep(time1);

	printf("Thread %d completing stage 1...\n", id);
	pthread_barrier_wait(&barrier1);

	int time2 = rand()%3 + 1;
        sleep(time2);

        printf("Thread %d completing stage 2...\n", id);
        pthread_barrier_wait(&barrier2);

	int time3 = rand()%3 + 1;
        sleep(time3);

        printf("Thread %d completing stage 3...\n", id);
        pthread_barrier_wait(&barrier3);

	return NULL;

}

int main(){
	srand(time(NULL));

	pthread_t th[M];
	int ids[M];
	pthread_barrier_init(&barrier1, NULL, M);
	pthread_barrier_init(&barrier2, NULL, M);
	pthread_barrier_init(&barrier3, NULL, M);

	for (int i = 0; i < M; i++) {
        	ids[i] = i;
        	pthread_create(&th[i], NULL, work, &ids[i]);
    	}

	for (int i = 0; i < M; i++) {
		pthread_join(th[i], NULL);
	}

	printf("All complete !");

	pthread_barrier_destroy(&barrier1);
	pthread_barrier_destroy(&barrier2);
	pthread_barrier_destroy(&barrier3);

	return 0;

}

