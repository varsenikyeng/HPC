#include <stdio.h>
#include <stdlib.h>

int main(){
	int *p = (int *)malloc(sizeof(int));
	if (p == NULL){
		printf("Malloc failed!");
		return 1;
	}
	*p = 10;
	printf("%d ", *p);
	printf("\n");
	int *arr = (int *)malloc(5*sizeof(int));
	if(arr == NULL){
		printf("Malloc failed!");
		return 1;
	}
	for(int i = 0; i < 5; i++){
		*(arr + i) = 10;
	}
	for(int i = 0; i < 5; i++){
		printf("%d ", *(arr + i));
	}

	free(p);
	free(arr);
}
