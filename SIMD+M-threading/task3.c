#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <immintrin.h>
#include <stdint.h>
#include <string.h>

#define N 4

typedef struct{
	unsigned char *input;
	unsigned char *output;
	size_t start;
	size_t end;
}Thread;

int width;
int height;
int maxval;

unsigned char* read_ppm(const char *filename){
	FILE *f = fopen(filename, "rb");
	if(!f){
		printf("Failed to open file\n");
		exit(1);
	}
	char format[3];
	fscanf(f, "%2s", format);

	if(strcmp(format, "P6") != 0){
		printf("Only P6 supportd\n");
		exit(1);
	}

	fscanf(f, "%d %d", &width, &height);
	fscanf(f, "%d", &maxval);
	fgetc(f);

	size_t size = width*height*3;
	unsigned char *data = malloc(size);
	fread(data, 1, size, f);
	fclose(f);

	return data;

}

void write_ppm(const char *filename, unsigned char *data){
	FILE *f = fopen(filename, "wb");
	fprintf(f, "P6\n%d %d\n%d\n", width, height, maxval);
	fwrite(data, 1, width*height*3, f);
	fclose(f);
}


void scalar_gray(unsigned char *input, unsigned char *output, size_t pixels){
	for(size_t i = 0; i < pixels; i++){
		unsigned char r = input[3*i];
		unsigned char g = input[3*i +1];
		unsigned char b = input[3*i +2];

		unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
		output[3*i] = gray;
		output[3*i + 1] = gray;
		output[3*i + 2] = gray;
	}
}

void* thread_scalar_gray(void *arg){
	Thread *data = (Thread*)arg;
	for(size_t i = data->start; i < data->end; i++){
		unsigned char r = data->input[3*i];
                unsigned char g = data->input[3*i +1];
                unsigned char b = data->input[3*i +2];
		unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
                data->output[3*i] = gray;
                data->output[3*i + 1] = gray;
                data->output[3*i + 2] = gray;
	}
	return NULL;
}

void simd_gray(unsigned char *input, unsigned char *output, size_t pixels){
	size_t i = 0;

	const __m256 fr = _mm256_set1_ps(0.299f);
	const __m256 fg = _mm256_set1_ps(0.587f);
	const __m256 fb = _mm256_set1_ps(0.114f);

	for(; i + 8 <= pixels; i += 8){
		float r[8], g[8], b[8];

		for(int j = 0; j < 8; j++){
			r[j] = input[3*(i+j)];
			g[j] = input[3*(i+j) + 1];
			b[j] = input[3*(i+j) + 2];
		}
		__m256 vr = _mm256_loadu_ps(r);
		__m256 vg = _mm256_loadu_ps(g);
		__m256 vb = _mm256_loadu_ps(b);

		__m256 gray = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(vr, fr), _mm256_mul_ps(vg, fg)), _mm256_mul_ps(vb, fb));
		float out[8];
		_mm256_storeu_ps(out, gray);

		for(int j = 0; j < 8; j++){
			unsigned char gval = (unsigned char)out[j];
			output[3*(i+j)] = gval;
			output[3*(i+j) + 1] = gval;
			output[3*(i+j) + 2] = gval;
		}
	}
	for(; i < pixels; i++){
		unsigned char r = input[3*i];
                unsigned char g = input[3*i +1];
                unsigned char b = input[3*i +2];

		unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);

		output[3*i] = gray;
                output[3*i + 1] = gray;
                output[3*i + 2] = gray;
	}
}

void* thread_simd_gray(void *arg){
	Thread *data = (Thread*)arg;
	size_t i = data->start;
	const __m256 fr = _mm256_set1_ps(0.299f);
        const __m256 fg = _mm256_set1_ps(0.587f);
        const __m256 fb = _mm256_set1_ps(0.114f);

	size_t limit = data->end - ((data->end - data->start)%8);
	for(; i < limit; i += 8){
		float r[8], g[8], b[8];

                for(int j = 0; j < 8; j++){
                        r[j] = data->input[3*(i+j)];
                        g[j] = data->input[3*(i+j) + 1];
                        b[j] = data->input[3*(i+j) + 2];
                }
		__m256 vr = _mm256_loadu_ps(r);
                __m256 vg = _mm256_loadu_ps(g);
                __m256 vb = _mm256_loadu_ps(b);

                __m256 gray = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(vr, fr), _mm256_mul_ps(vg, fg)), _mm256_mul_ps(vb, fb));
                float out[8];
                _mm256_storeu_ps(out, gray);

                for(int j = 0; j < 8; j++){
                        unsigned char gval = (unsigned char)out[j];
                        data->output[3*(i+j)] = gval;
                        data->output[3*(i+j) + 1] = gval;
                        data->output[3*(i+j) + 2] = gval;
                }
	}

	for(; i < data->end; i++){
                unsigned char r = data->input[3*i];
                unsigned char g = data->input[3*i +1];
                unsigned char b = data->input[3*i +2];

                unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);

                data->output[3*i] = gray;
                data->output[3*i + 1] = gray;
                data->output[3*i + 2] = gray;
        }
	return NULL;
}

int main(){
	unsigned char *input = read_ppm("input.ppm");
	size_t pixels = width * height;
	
	unsigned char *out_scalar = malloc(pixels*3);
	unsigned char *out_simd = malloc(pixels*3);
	unsigned char *out_mt = malloc(pixels*3);
	unsigned char *out_both = malloc(pixels*3);

	pthread_t threads[N];
	Thread t[N];

	struct timespec start, end;
	double passed;

	printf("Image size : %d x %d\n", width, height);
	printf("Threads used: %d\n\n", N);

	clock_gettime(CLOCK_MONOTONIC, &start);
	scalar_gray(input, out_scalar, pixels);
	clock_gettime(CLOCK_MONOTONIC, &end);

	passed = (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1e9;
	printf("Scalar time : %f sec \n", passed);

	clock_gettime(CLOCK_MONOTONIC, &start);
	simd_gray(input, out_simd, pixels);
	clock_gettime(CLOCK_MONOTONIC, &end);
	passed = (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1e9;
	printf("SIMD time : %f sec \n", passed);

	size_t chunk = pixels/N;
	clock_gettime(CLOCK_MONOTONIC, &start);

	for(int i = 0; i < N; i++){
		t[i].input = input;
		t[i].output = out_mt;
		t[i].start = i*chunk;
		t[i].end = (i == N-1)?pixels:(i+1)*chunk;
		pthread_create(&threads[i], NULL, thread_scalar_gray, &t[i]);
	}

	for(int i =0; i <N; i++)
		pthread_join(threads[i], NULL);

	clock_gettime(CLOCK_MONOTONIC, &end);
	passed = (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1e9;
        printf("Multithreading time : %f sec \n", passed);
	
	clock_gettime(CLOCK_MONOTONIC, &start);

        for(int i = 0; i < N; i++){
                t[i].input = input;
                t[i].output = out_both;
                t[i].start = i*chunk;
                t[i].end = (i == N-1)?pixels:(i+1)*chunk;
                pthread_create(&threads[i], NULL, thread_simd_gray, &t[i]);
        }

	for(int i =0; i <N; i++)
                pthread_join(threads[i], NULL);

	clock_gettime(CLOCK_MONOTONIC, &end);
        passed = (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1e9;
        printf("Multithreading + SIMD time : %f sec \n", passed);

	write_ppm("gray_output.ppm", out_both);

	printf("\nOutput image: gray_output.ppm\n");
	free(input);
	free(out_scalar);
	free(out_simd);
	free(out_mt);
	free(out_both);
	
	return 0;
}



