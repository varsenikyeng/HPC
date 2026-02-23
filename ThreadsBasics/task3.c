#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 50000000
#define N 4

int *array;

typedef struct{
	int id;
	int max_value;
} ThreadResult;

ThreadResult results[N];

void* find_max(void* arg){
	int id = *((int*)arg);
	int chunk = SIZE / N;
	int start = id * chunk;
	int end;
	if (id == N - 1) {
		end = SIZE;
	} else {
		end = start + chunk;
	}
	
	results[id].id = id;
	results[id].max_value = array[start];
	
	for (int i = start + 1; i < end; i++){
		if (array[i] > results[id].max_value) {
			results[id].max_value = array[i];
		}
	}
	
	return NULL;
}

int main(){
	srand(time(NULL));
	
	array = malloc(sizeof(int) * SIZE);
	
	if (array == NULL) {
		printf("Memory allocation failed!\n");
		return 1;
	}
	
	for(int i = 0; i < SIZE; i++){
		array[i] = rand()%1000;
	}
	
	clock_t start = clock();
	int seq_max = array[0];
	for(int i = 1; i < SIZE; i++){
		if (array[i] > seq_max) {
			seq_max = array[i];
		}
	}
	
	clock_t end = clock();
	printf("Sequential Maximum: %d\n", seq_max);
	printf("Sequential Time: %f sec\n", (double)(end - start) / CLOCKS_PER_SEC);
	
	pthread_t threads[N];
	int ids[N];
	start = clock();
	
	for(int i = 0; i < N; i++){
		ids[i] =i;
		pthread_create(&threads[i], NULL, find_max, &ids[i]);
	}
		
	for(int i = 0; i < N; i++){
		pthread_join(threads[i], NULL);
	}

	int global_max = results[0].max_value;

	for(int i = 1; i < N; i++){
		if (results[i].max_value > global_max) {
			global_max = results[i].max_value;
		}
	}
	
	end = clock();
	
	printf("\nGlobal Maximum in parallel: %d\n", global_max);
	printf("Parallel time: %f sec\n", (double)(end - start) / CLOCKS_PER_SEC);
       
	free(array);
	
	return 0;
}
