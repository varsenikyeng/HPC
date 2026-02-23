#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 50000000
#define N 5

int *array;

void* compute_sum(void* arg){
	int id = *((int*)arg);

	int chunk = SIZE / N;
	int start = id * chunk;
	int end = start + chunk;
	long* partial_sum = malloc(sizeof(long));
    	*partial_sum = 0;
	for (int i = start; i < end; i++){
		*partial_sum += array[i];
	}

	return partial_sum;
}
int main(){

	srand(time(NULL));

	array = malloc(sizeof(int)*SIZE);
	if (array == NULL) {
        	printf("Memory allocation failed!\n");
        	return 1;
	}

	for(int i = 0; i < SIZE; i++){
		array[i] = rand()%10;
	}

	clock_t start = clock();

	long seq_sum = 0;
	for(int i = 0; i < SIZE; i++){
		seq_sum += array[i];
	}

	clock_t end = clock();
	printf("Sequential Sum: %ld\n", seq_sum);
	printf("Sequential Time: %f sec\n",(double)(end - start) / CLOCKS_PER_SEC);

	pthread_t threads[N];
	int ids[N];
	start = clock();

	for(int i = 0; i < N; i++){
		ids[i] = i;
		pthread_create(&threads[i], NULL, compute_sum, &ids[i]);
	}

	long total = 0;

	for(int i = 0; i < N; i++){
		long* partial;
		pthread_join(threads[i], (void**)&partial);
		total += *partial;
		free(partial);
	}

	end = clock();

	printf("Parallel Sum: %ld\n", total);
    	printf("Parallel Time: %f sec\n",(double)(end - start) / CLOCKS_PER_SEC);
	free(array);
	
	return 0;
}
