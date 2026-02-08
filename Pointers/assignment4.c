#include <stdio.h>

int main(){
	int x = 5;
	int *ptr = &x;
	int **pptr = &ptr;
	printf("%d\n", *ptr);
	printf("%d", **pptr);
}
