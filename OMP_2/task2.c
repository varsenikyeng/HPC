#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>

#define N 50000000

int main(){
	double *A = malloc(sizeof(double) * N);
	for(int i = 0; i < N; i++){
		A[i] = ((double)rand() / RAND_MAX)*50;
	}
	double min_diff = 1e8;

	#pragma omp parallel for reduction(min: min_diff)
	for(int i = 1; i < N; i++){
		double diff = fabs(A[i] - A[i-1]);
		if(diff < min_diff){
			min_diff = diff;
		}
	}
	printf("Minimum difference : %.10f\n", min_diff);

	free(A);
	return 0;
}
