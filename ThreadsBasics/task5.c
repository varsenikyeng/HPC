
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

#define N 8
#define WORK 100000000

void* heavy_work(void* arg){
	int id = *((int*)arg);
	unsigned long long sum = 0;
	for(unsigned long long i = 0; i < WORK; i++){
		sum += i % 3;
	}
	printf("Thread %d running on CPU %d, sum %llu\n", id, sched_getcpu(), sum);
	return NULL;
}

int main(){
	pthread_t threads[N];
	int ids[N];
	for(int i = 0; i < N; i++){
		ids[i] = i;
		pthread_create(&threads[i], NULL, heavy_work, &ids[i]);
	}
	for(int i = 0; i < N; i++)
		pthread_join(threads[i], NULL);

	return 0;
}
