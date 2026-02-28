#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define S 5

pthread_barrier_t barrierSensor;

double temperatures[S];
double avg = 0;

void* sensor(void* arg){
	int id = *(int*)arg;
	int time = rand() % 3 + 1;
	sleep(time);

	double temp = 10 + (rand() % 150)/10.0;
	temperatures[id] = temp;

	printf("Waiting ...\n");
	pthread_barrier_wait(&barrierSensor);
	return NULL;
}

int main(){

	srand(time(NULL));

	pthread_t th[S];
	int ids[S];

	pthread_barrier_init(&barrierSensor, NULL, S);

	for(int i = 0; i < S; i++){
		ids[i] = i;
		pthread_create(&th[i], NULL, sensor, &ids[i]);
	}

	for(int i = 0; i < S; i++){
		pthread_join(th[i], NULL);
	}

	double sum = 0;
	for(int i = 0; i < S; i++){
		sum+= temperatures[i];
	}

	avg = sum/S;

	printf("\nAll sesnors finished.\n");
	printf("Average temperature: %.2f °C\n", avg);
	
	pthread_barrier_destroy(&barrierSensor);
	return 0;

}
