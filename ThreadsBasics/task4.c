#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 20000000
#define N 4

typedef struct{
	int id;
	int count;
} ThreadResult;

ThreadResult results[N];

int is_prime(int num){
	if(num < 2)
		return 0;
	if(num == 2)
		return 1;
	if(num%2 == 0) return 0;
	int sq = (int)sqrt(num);
	for(int i = 3; i <= sq; i += 2){
		if(num % i == 0)
			return 0;
	}
	return 1;
}

void* count_primes(void* arg){
	int id = *((int*)arg);
	int chunk = SIZE / N;
	int start = id * chunk + 1;
	int end = start + chunk -1;
	if(id == N-1)
		end = SIZE;

	results[id].id = id;
	results[id].count = 0;
	for(int i = start; i <= end; i++){
		if(is_prime(i)){
			results[id].count++;
		}
	}
	return NULL;
}

int main(){
	clock_t start = clock();
	int seq_count = 0;
	for(int i = 1; i <= SIZE; i++){
		if(is_prime(i)){
			seq_count++;
		}
	}
	clock_t end = clock();
	printf("Sequential Prime Count: %d\n", seq_count);
	printf("Sequential Time: %f sec\n\n",(double)(end-start)/CLOCKS_PER_SEC);

	pthread_t threads[N];
	int ids[N];
	start = clock();
	for(int i = 0; i < N; i++){
		ids[i] = i;
		pthread_create(&threads[i], NULL, count_primes, &ids[i]);
	}
	for(int i = 0; i < N; i++){
		pthread_join(threads[i], NULL);
	}
	
	int global_count = 0;
	for(int i = 0; i < N; i++)
		global_count += results[i].count;

	end = clock();
	printf("Parallel prime count: %d\n", global_count);
	printf("Parallel time: %f secs\n",(double)(end-start)/CLOCKS_PER_SEC);
	return 0;
}

