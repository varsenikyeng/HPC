#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void* print_message(void* arg){
	int thread_id = *((int*)arg);
	printf("Thread %d is running... \n", thread_id);
}

int main(){
	pthread_t threads[3];
	int thread_ids[3];

	for (int i = 0; i < 3; i++){
		thread_ids[i] = i;
		pthread_create(&threads[i], NULL, print_message, (void*)&thread_ids[i]);
		
	}

	for(int i = 0; i< 3; i++){
		pthread_join(threads[i], NULL);
	}

	printf("All threads completed\n");

	return 0;

}
