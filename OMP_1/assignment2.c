#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 10000

typedef struct{
	int order_id;
	int distance_km;
	int priority;
}Order;

int main(){
	Order orders[N];
	int threshold;
	int thread_high_count[4] = {0};

	for(int i = 0; i < N; i++){
		orders[i].order_id = i;
		orders[i].distance_km = rand()%40;
		orders[i].priority = 0;
	}

	#pragma omp parallel num_threads(4)
	{
		int id = omp_get_thread_num();

		#pragma omp single
		{
			threshold = 20;
		}
		#pragma omp for
		for(int i = 0; i < N; i++){
			if(orders[i].distance_km < threshold)
				orders[i].priority = 1;
			else
				orders[i].priority = 0;
		}

		#pragma omp barrier

		#pragma omp single
		{
			printf("Priority assignment is finished.\n");
		}

		#pragma omp for
		for(int i = 0; i < N; i++){
			if(orders[i].priority == 1){
				thread_high_count[id]++;
			}
		}

		#pragma omp barrier

		#pragma omp single
		{
			int total = 0;
			for(int i = 0; i < 4; i++){
				printf("Thread %d : %d\n", i, thread_high_count[i]);
				total+= thread_high_count[i];
			}
			printf("Total HIGH priority orders : %d", total);
		}
	}
	return 0;
}
