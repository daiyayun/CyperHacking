#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void){
	char* str = (char* )malloc(sizeof(char)*10);

	strcpy(str, "hello");
	puts(str);
	printf("%lu\n", sizeof(str));
	printf("%lu\n", strlen(str));
	printf("%c\n", str[2]);
}