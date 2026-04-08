#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

#define N 100000

int main(){
	double *A = malloc(sizeof(double) * N);
	double maxA = 0;
	for(int i = 0; i < N; i++){
		A[i] = ((double)rand()/RAND_MAX) * 1000.0;
	}	
	#pragma omp parallel for reduction(max:maxA)
	for(int i = 0; i < N; i++){
		double a = A[i];
		maxA = a;
	}
	double T = 0.8 * maxA;
	double sum = 0.0;
	#pragma omp parallel for reduction(+:sum)
	for(int i = 0; i < N; i++){
		double b = A[i];
		if(b > T)
			sum+= b;
	}
	printf("Sum : %.6f\n", sum);
	free(A);
	return 0;

}
