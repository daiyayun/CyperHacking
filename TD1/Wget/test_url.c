#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "test_url.h"

int main(int argc, char* argv[]){
	char* url = argv[1];
	url_info info = parseURL(url);
	bool v = validateURL(info);
	if(v){
		printf("%s is a valid url.\n", url);
		printf("protocol: %s\n", p.protocol);
		printf("hostname: %s\n", p.hostname);
		printf("port: %d\n", p.port);
		printf("path: %s\n", p.path);
	}
	else{
		printf("%s is an invalid url.\n", url);
	}
}

url_info parseURL(url){
	url_info info;
	info.protocol = strtok(url, ":");
	char* h = strtok(NULL, ":");
	info.port = atoi(strtok(NULL, "/"));
	info.path = strtok(NULL, "/");
	info.hostname = strtok(h, "/");
}

bool validateURL(info){

}

