#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int fib(int n){
	if(n <= 1){
		return n;
	}
	if(n<= 10){
		return fib(n-2) + fib(n-1);
	}

	int a, b;
	#pragma omp task shared(a)
	{
		a = fib(n-1);
	}

	#pragma omp task shared(b)
	{
		b = fib(n-2);
	}

	#pragma omp taskwait

	return a + b;
}

int main(){

	int n;
	printf("Enter n : ");
	scanf("%d", &n);

	int result;

	#pragma omp parallel
	{
		#pragma omp single
		{
			result = fib(n);
		}
		
	}
	printf("Fib(%d) = %d\n", n, result);

	return 0;
}
