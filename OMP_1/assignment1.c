#include <stdlib.h>
#include <stdio.h>
#include <omp.h>


#define N 20000

typedef struct{
	int request_id;
	int user_id;
	int response_time_ms;
}LogEntry;

int main(){
	LogEntry entries[N];
	int th_fast[4] = {0};
	int th_medium[4] = {0};
	int th_slow[4] = {0};

	#pragma omp parallel num_threads(4)
	{

		int id = omp_get_thread_num();
		#pragma omp single
		{
			for(int i = 0; i < N; i++){
				entries[i].request_id = i;
				entries[i].user_id = rand()%500;
				entries[i].response_time_ms = rand()% 600;

			}
		}

		#pragma omp barrier

		#pragma omp for
		for (int i = 0; i < N; i++){
			int t = entries[i].response_time_ms;
			if(t<100){
				th_fast[id]++;
			}
			else if(t <= 300){
				th_medium[id]++;
			}
			else {
				th_slow[id]++;
			}
		}
		#pragma omp barrier

		#pragma omp single
		{
			int fast = 0;
			int medium = 0;
			int slow = 0;
			for(int i = 0; i < 4; i++){
				fast += th_fast[i];
				medium += th_medium[i];
				slow += th_slow[i];
			}
			printf("FAST : %d\n", fast);
			printf("MEDIUM: %d\n", medium);
			printf("SLOW : %d\n", slow);
		}

	}
	return 0;
	
}
