#include <stdio.h>

int str_length(char *str){
	int length = 0;
	int i = 0;
	while(*(str + i) != '\0'){
		length++;
		i++;
	}
	return length;
}

int main(){
	char *str = "Hellooo";
	char *ptr = str;
	while(*ptr != '\0'){
		printf("%c", *ptr);
		*ptr++;
	}
	printf("\nEnter a string please...\n");
	char *input;
	scanf("%s", input);
	printf("%d", str_length(input));



}
