#include <stdio.h>

int main(){
	int var = 10;
	int *ptr;
	ptr = &var;
	printf("Address using the variable: %p \n", &var);
	printf("Address using the pointer: %p \n", ptr);
	*ptr = 20;
	printf("New value : %d \n", var);
}
