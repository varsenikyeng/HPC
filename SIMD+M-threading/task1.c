#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <immintrin.h>
#include <stdint.h>

#define DNA_SIZE (256 * 1024 * 1024) // 256 MB
#define N 4

typedef struct{
    const unsigned char *dna;
    size_t start;
    size_t end;
    uint64_t local_counts[4];
}Thread;

uint64_t global_counts[4] = {0, 0, 0, 0}; //A C G T
pthread_mutex_t mutex;

void generate_seq(char *buffer, size_t size){
	const char nucleotides[4] = {'A', 'C', 'G', 'T'};
	for (size_t i =0; i < size; i++){
		buffer[i] = nucleotides[rand()%4];
	}
}

void scalar_count(char *dna, size_t size, uint64_t *counts){
	counts[0] = 0;
	counts[1] = 0;
	counts[2] = 0;
	counts[3] = 0;
	for(size_t i = 0; i < size; i++){
		switch(dna[i]){
			case 'A': 
				counts[0]++;
				break;
			case 'C':
				counts[1]++;
				break;
			case 'G':
				counts[2]++;
				break;
			case 'T':
				counts[3]++;
				break;
		}
	}
}

void* multithread_count(void *arg){
	Thread *data = (Thread*)arg;
	uint64_t *local = data->local_counts;
	local[0] = 0;
	local[1] = 0;
	local[2] = 0;
	local[3] = 0;

	for(size_t i = data->start; i < data->end; i++){
		switch(data->dna[i]){
			case 'A':
                                local[0]++;
                                break;
                        case 'C':
                                local[1]++;
                                break;
                        case 'G':
                                local[2]++;
                                break;
                        case 'T':
                                local[3]++;
                                break;
			
		}
	}
	pthread_mutex_lock(&mutex);
	for(int j = 0; j < 4; j++)
		global_counts[j]+= local[j];
	pthread_mutex_unlock(&mutex);

	return NULL;
}

void simd_count(const unsigned char *dna, size_t size, uint64_t counts[4]){
	counts[0] = 0;
	counts[1] = 0;
	counts[2] = 0;
	counts[3] = 0;
	size_t i = 0;

	const __m256i vA = _mm256_set1_epi8('A');
	const __m256i vC = _mm256_set1_epi8('C');
	const __m256i vG = _mm256_set1_epi8('G');
	const __m256i vT = _mm256_set1_epi8('T');

	size_t limit = size& ~(size_t)31;

	for(; i < limit; i+= 32){
		__m256i chunk = _mm256_loadu_si256((const __m256i*)(dna + i));

		__m256i eqA = _mm256_cmpeq_epi8(chunk, vA);
		__m256i eqC = _mm256_cmpeq_epi8(chunk, vC);
        	__m256i eqG = _mm256_cmpeq_epi8(chunk, vG);
        	__m256i eqT = _mm256_cmpeq_epi8(chunk, vT);

		unsigned maskA = _mm256_movemask_epi8(eqA);
		unsigned maskC = _mm256_movemask_epi8(eqC);
		unsigned maskG = _mm256_movemask_epi8(eqG);
		unsigned maskT = _mm256_movemask_epi8(eqT);

		counts[0] += __builtin_popcount(maskA);
		counts[1] += __builtin_popcount(maskC);
		counts[2] += __builtin_popcount(maskG);
		counts[3] += __builtin_popcount(maskT);
	}

	for(; i < size; i++){
		switch(dna[i]){
                        case 'A':
                                counts[0]++;
                                break;
                        case 'C':
                                counts[1]++;
                                break;
                        case 'G':
                                counts[2]++;
                                break;
                        case 'T':
                                counts[3]++;
                                break;

                }
	}
}

void* thread_simd_count(void *arg){
	Thread *data = (Thread*)arg;
	uint64_t *local = data->local_counts;
        local[0] = 0;
        local[1] = 0;
        local[2] = 0;
        local[3] = 0;

	const __m256i vA = _mm256_set1_epi8('A');
	const __m256i vC = _mm256_set1_epi8('C');
	const __m256i vG = _mm256_set1_epi8('G');
	const __m256i vT = _mm256_set1_epi8('T');

	size_t i = data->start;
	size_t limit = data->end - ((data->end - data->start)%32);
	for(; i < limit; i+= 32){
		__m256i chunk = _mm256_loadu_si256((const __m256i*)(data->dna+i));

		__m256i eqA = _mm256_cmpeq_epi8(chunk, vA);
                __m256i eqC = _mm256_cmpeq_epi8(chunk, vC);
                __m256i eqG = _mm256_cmpeq_epi8(chunk, vG);
                __m256i eqT = _mm256_cmpeq_epi8(chunk, vT);

		unsigned maskA = _mm256_movemask_epi8(eqA);
                unsigned maskC = _mm256_movemask_epi8(eqC);
                unsigned maskG = _mm256_movemask_epi8(eqG);
                unsigned maskT = _mm256_movemask_epi8(eqT);

                local[0] += __builtin_popcount(maskA);
                local[1] += __builtin_popcount(maskC);
                local[2] += __builtin_popcount(maskG);
                local[3] += __builtin_popcount(maskT);
	}

	for(; i < data->end; i++){
                switch(data->dna[i]){
                        case 'A':
                                local[0]++;
                                break;
                        case 'C':
                                local[1]++;
                                break;
                        case 'G':
                                local[2]++;
                                break;
                        case 'T':
                                local[3]++;
                                break;

                }
        }
	pthread_mutex_lock(&mutex);

	global_counts[0] += local[0];
	global_counts[1] += local[1];
	global_counts[2] += local[2];
	global_counts[3] += local[3];

	pthread_mutex_unlock(&mutex);
	return NULL;

}

int main(){
	srand(time(NULL));
	unsigned char *dna = malloc(DNA_SIZE);

	if(!dna){
		printf("Malloc failed!\n");
		return 1;
	}

	generate_seq(dna, DNA_SIZE);

	printf("DNA size : %lu MB\n", (unsigned long)(DNA_SIZE/(1024*1024)));
	printf("Threads used: %d\n", N);

	uint64_t counts[4];

	struct timespec start, end;
	double passed;


	clock_gettime(CLOCK_MONOTONIC, &start);
	scalar_count(dna, DNA_SIZE, counts);

	clock_gettime(CLOCK_MONOTONIC, &end);

	passed = (end.tv_sec - start.tv_sec) +
              (end.tv_nsec - start.tv_nsec) / 1e9;

	printf("Counts (A C G T): %lu %lu %lu %lu\n",
           counts[0], counts[1], counts[2], counts[3]);
	
	printf("Scalar time: %f sec\n\n", passed);

	pthread_t threads[N];
	Thread t[N];
	for (int i = 0; i < 4; i++)
        	global_counts[i] = 0;
	
	size_t chunk = DNA_SIZE / N;
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	for (int i = 0; i < N; i++) {
		t[i].dna = dna;
		t[i].start = i*chunk;
		t[i].end = (i == N - 1) ? DNA_SIZE : (i + 1) * chunk;
		pthread_create(&threads[i], NULL, multithread_count, &t[i]);
	}
	
	for (int i = 0; i < N; i++)
		pthread_join(threads[i], NULL);
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	passed = (end.tv_sec - start.tv_sec) +
              (end.tv_nsec - start.tv_nsec) / 1e9;
	
	printf("Counts (A C G T): %lu %lu %lu %lu\n",global_counts[0], global_counts[1], global_counts[2], global_counts[3]);
	
	printf("Multithreading time: %f sec\n\n", passed);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	simd_count(dna, DNA_SIZE, counts);
	clock_gettime(CLOCK_MONOTONIC, &end);
	passed = (end.tv_sec - start.tv_sec) +
              (end.tv_nsec - start.tv_nsec) / 1e9;
	
	printf("Counts (A C G T): %lu %lu %lu %lu\n", counts[0], counts[1], counts[2], counts[3]);
	
	printf("SIMD time: %f sec\n\n", passed);
	
	for (int i = 0; i < 4; i++)
		global_counts[i] = 0;
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	for (int i = 0; i < N; i++) {
		t[i].dna = dna;
		t[i].start = i * chunk;
		t[i].end = (i == N - 1) ? DNA_SIZE : (i + 1) * chunk;
		
		pthread_create(&threads[i], NULL, thread_simd_count, &t[i]);
	}

	for (int i = 0; i < N; i++)
        	pthread_join(threads[i], NULL);
	
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	passed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
	
	printf("Counts (A C G T): %lu %lu %lu %lu\n", global_counts[0], global_counts[1], global_counts[2], global_counts[3]);
	
	printf("SIMD + Multithreading time: %f sec\n", passed);
	free(dna);
	
	return 0;
}
