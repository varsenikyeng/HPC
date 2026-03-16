#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <immintrin.h>
#include <stdint.h>
#include <string.h>

#define BUF_SIZE (256*1024*1024)
#define N 4

typedef struct{
	unsigned char *buf;
	size_t start;
	size_t end;
} Thread;

void generate_buffer(unsigned char *buffer, size_t size){
	for(size_t i = 0; i < size; i++){
		int r = rand()%5;

		if(r == 0)
			buffer[i] = 'a' + rand()%26;
		else if(r == 1)
			buffer[i] = 'A' + rand()%26;
		else if(r == 2)
			buffer[i] = '0' + rand()%10;
		else if(r == 3)
			buffer[i] = "!?,.;:"[rand()%6];
		else
			buffer[i] = ' ';
	}
}

void* multithread_convert(void *arg){
	Thread *data = (Thread*)arg;

	for(size_t i = data->start; i < data->end; i++){
		if(data->buf[i] >= 'a' && data->buf[i] <= 'z')
			data->buf[i] -= 32;
	}
	return NULL;
}

void simd_convert(unsigned char *buf, size_t size){
	size_t i = 0;
	const __m256i va = _mm256_set1_epi8('a');
	const __m256i vz = _mm256_set1_epi8('z');
	const __m256i v32 = _mm256_set1_epi8(32);

	size_t limit = size&~(size_t)31;

	for(; i < limit; i+= 32){
		__m256i chunk = _mm256_loadu_si256((__m256i*)(buf + i));

		__m256i ge_a = _mm256_cmpgt_epi8(chunk, _mm256_sub_epi8(va, _mm256_set1_epi8(1)));
		__m256i le_z = _mm256_cmpgt_epi8(_mm256_add_epi8(vz, _mm256_set1_epi8(1)), chunk);
		__m256i mask = _mm256_and_si256(ge_a, le_z);
		__m256i sub = _mm256_and_si256(mask, v32);

		chunk = _mm256_sub_epi8(chunk, sub);
		_mm256_storeu_si256((__m256i*)(buf + i), chunk);
	}
	for(; i < size; i++){
		if (buf[i] >= 'a' && buf[i] <= 'z')
			buf[i] -= 32;
	}
}

void* thread_simd_convert(void *arg){
	Thread *data = (Thread*)arg;

	size_t i = data->start;

	const __m256i va = _mm256_set1_epi8('a');
	const __m256i vz = _mm256_set1_epi8('z');
        const __m256i v32 = _mm256_set1_epi8(32);

	size_t limit = data->end - ((data->end - data->start)%32);
	for(; i < limit; i+=32){
		__m256i chunk = _mm256_loadu_si256((__m256i*)(data->buf + i));
		__m256i ge_a = _mm256_cmpgt_epi8(chunk, _mm256_sub_epi8(va, _mm256_set1_epi8(1)));
                __m256i le_z = _mm256_cmpgt_epi8(_mm256_add_epi8(vz, _mm256_set1_epi8(1)), chunk);
                __m256i mask = _mm256_and_si256(ge_a, le_z);
                __m256i sub = _mm256_and_si256(mask, v32);

                chunk = _mm256_sub_epi8(chunk, sub);
                _mm256_storeu_si256((__m256i*)(data->buf + i), chunk);
	}
	for(; i < data->end; i++){
                if (data->buf[i] >= 'a' && data->buf[i] <= 'z')
                        data->buf[i] -= 32;
        }
	return NULL;
}

int main(){
	srand(time(NULL));

	unsigned char *buffer = malloc(BUF_SIZE);
	unsigned char *buf_mt = malloc(BUF_SIZE);
	unsigned char *buf_simd = malloc(BUF_SIZE);
	unsigned char *buf_both = malloc(BUF_SIZE);

	if(!buffer || !buf_mt || !buf_simd || !buf_both){
		printf("Malloc failed\n");
		return 1;
	}

	 generate_buffer(buffer, BUF_SIZE);

	 memcpy(buf_mt, buffer, BUF_SIZE);
	 memcpy(buf_simd, buffer, BUF_SIZE);
	 memcpy(buf_both, buffer, BUF_SIZE);

	 printf("Buffer size : %lu MB\n", (unsigned long)(BUF_SIZE/(1024*1024)));
	 printf("Threads used: %d\n\n", N);
	 pthread_t threads[N];
	 Thread t[N];
	 struct timespec start, end;
	 double passed;
	 size_t chunk = BUF_SIZE / N;
	
	 clock_gettime(CLOCK_MONOTONIC, &start);

	 for(int i = 0; i < N; i++){
	 	t[i].buf = buf_mt;
		t[i].start = i*chunk;
		t[i].end = (i == N - 1) ? BUF_SIZE : (i+1)*chunk;
		pthread_create(&threads[i], NULL, multithread_convert, &t[i]);
	 }

	 for(int i = 0; i < N; i++){
	 	pthread_join(threads[i], NULL);
	 }

	 clock_gettime(CLOCK_MONOTONIC, &end);
	 passed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

	 printf("Multithreading time: %f sec\n", passed);

	 clock_gettime(CLOCK_MONOTONIC, &start);

	 simd_convert(buf_simd, BUF_SIZE);

	 clock_gettime(CLOCK_MONOTONIC, &end);
         passed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

	 printf("SIMD time: %f sec \n", passed);

	 clock_gettime(CLOCK_MONOTONIC, &start);

	 for(int i = 0; i < N; i++){
	 	t[i].buf = buf_both;
		t[i].start = i * chunk;
		t[i].end = (i == N-1) ? BUF_SIZE : (i+1)*chunk;

		pthread_create(&threads[i], NULL, thread_simd_convert, &t[i]);
	 }

	 for(int i = 0; i < N; i++){
                pthread_join(threads[i], NULL);
         }
	 clock_gettime(CLOCK_MONOTONIC, &end);
         passed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
	 printf("SIMD + Multithreading time: %f sec \n", passed);

	 free(buffer);
	 free(buf_mt);
	 free(buf_both);
	 free(buf_simd);

	 return 0;

}
