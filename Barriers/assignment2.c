#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define N 5

pthread_barrier_t lobbyBarrier;

void* player(void* args){
	int id = *(int*)args;
	free(args);

	int time = rand()%3 + 1;
	printf("Player %d gettingn ready...\n", id);
	sleep(time);

	pthread_barrier_wait(&lobbyBarrier);
	printf("Game Started!\n");

	return NULL;
}

int main(){
	srand(time(NULL));
	pthread_t th[N];

	pthread_barrier_init(&lobbyBarrier, NULL, N);

	for(int i = 0; i < N; i++){
		int* id = malloc(sizeof(int));
		*id = i;
		pthread_create(&th[i], NULL, player, id);
	}

	for(int i = 0; i < N; i++){
		pthread_join(th[i], NULL);
	}

	pthread_barrier_destroy(&lobbyBarrier);

	return 0;
}
