#include <stdio.h>
#include <string.h>
#include "fancy-hello-world.h"

int main(void) {
	char name[] = "Yayun";
	char output[] = "Hello World, hello ";
    hello_string(name, output); 
    return 0;
}

void hello_string(char* name, char* output){
	strcat(output, name); 
	printf("%s", output);
}