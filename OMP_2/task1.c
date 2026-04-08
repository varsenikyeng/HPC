#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100000000

int main(){
	int *A = malloc(sizeof(int) * N);
	int hist_naive[256] = {0};
	int hist_critical[256] = {0};
	int hist_reduction[256] = {0};
	for(int i = 0; i < N; i++){
		A[i] = rand()%256;
	}
	#pragma omp parallel for
	for(int i = 0; i < N; i++){
			int a = A[i];
			hist_naive[a]++;
	}

	#pragma omp parallel for
	for(int i = 0; i < N; i++){
		int a = A[i];
		#pragma omp critical
		{
			hist_critical[a]++;
		}
	}

	#pragma omp parallel for reduction(+:hist_reduction[:256])
	for(int i = 0; i < N; i++){
		int a = A[i];
		hist_reduction[a]++;
	}

	printf("Value 0 naive = %d critical = %d reduction = %d\n",hist_naive[0], hist_critical[0], hist_reduction[0]);
	printf("Value 150 naive = %d critical = %d reduction = %d\n", hist_naive[150], hist_critical[150], hist_reduction[150]);
	printf("Value 255 naive = %d critical = %d reduction = %d\n", hist_naive[255], hist_critical[255], hist_reduction[255]);


	free(A);
	return 0;
	
	
}
