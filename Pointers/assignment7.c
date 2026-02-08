#include <stdio.h>

int main(){
	char *strings[] = {"Alo", "Hello", "Apple"};
	for(int i = 0; i < 3; i++){
		printf("%s\n", *(strings + i));
	}

	char newString[] = "Homework";
	strings[1] = newString;
	printf("After modification:\n");
	for(int i = 0; i < 3; i++){
                printf("%s\n", *(strings + i));
        }
}
