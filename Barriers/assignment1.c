#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define P 5
#define R 3

int dice_values[P];
int wins[P] = {0};
int status[P] = {0};

pthread_barrier_t barrierRolledDice;
pthread_barrier_t barrierCalculated;
pthread_barrier_t barrierPrinted;

void* roll(void* args) {
	int index = *(int*)args;
	free(args);
	for (int r = 0; r < R; r++){
		dice_values[index] = rand() % 6 + 1;
		pthread_barrier_wait(&barrierRolledDice);
		pthread_barrier_wait(&barrierCalculated);

		if(status[index] == 1){
			printf("Player %d won\n", index);
		}
		pthread_barrier_wait(&barrierPrinted);
	}
}

int main(int argc, char *argv[]){
	srand(time(NULL));
	pthread_t th[P];
	int i;
	pthread_barrier_init(&barrierRolledDice, NULL, P + 1);
	pthread_barrier_init(&barrierCalculated, NULL, P + 1);
	pthread_barrier_init(&barrierPrinted, NULL, P + 1);
	for(i = 0; i < P; i++){
		int* a = malloc(sizeof(int));
		*a = i;
		if(pthread_create(&th[i], NULL, &roll, (void*) a) != 0){
			perror("Failed to create thread");
		}
	}
	for (int r = 0; r < R; r++){
		pthread_barrier_wait(&barrierRolledDice);
		int max = 0;
		for(i = 0; i < P; i++){
			if(dice_values[i] > max){
				max = dice_values[i];
			}
		}
		printf("Round %d:\n", r + 1);
		for (int i = 0; i < P; i++){
			printf("Player %d rolled %d\n", i, dice_values[i]);
			if (dice_values[i] == max){
				wins[i]++;
				status[i] = 1;
			}else {
				status[i] = 0;
			}
		}
		pthread_barrier_wait(&barrierCalculated);
		pthread_barrier_wait(&barrierPrinted);
	}

	int all = 0;
	for (int i = 1; i < P; i++)
		if(wins[i] > wins[all])
			all = i;
	printf("\nOverall winner: Player %d\n", all);

	for (i = 0; i < P; i++){
		if(pthread_join(th[i], NULL) != 0){
			perror("Failed to join thread");
		}
	}

	pthread_barrier_destroy(&barrierRolledDice);
	pthread_barrier_destroy(&barrierCalculated);
	pthread_barrier_destroy(&barrierPrinted);
	return 0;
	
}
