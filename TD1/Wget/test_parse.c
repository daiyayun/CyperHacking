#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
	char* url = argv[1];
	printf("%s\n", strtok(url, ":"));
	char* h = strtok(NULL, ":");
	
	printf("%d\n", atoi(strtok(NULL, "/")));
	printf("%s\n", strtok(NULL, "."));
	printf("%s\n", strtok(h, "/"));

	return 0;
}