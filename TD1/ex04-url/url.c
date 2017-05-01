/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"url.h"



/**
 * parse a URL and store the information in info.
 */

void parse_url(char* url, url_info *info)
{
	// url format: [http://]<hostname>[:<port>]/<path>
	// e.g. https://www.polytechnique.edu:80/index.php
	(*info).url = url;
	(*info).protocol = strtok(url, ":");
	char* rest;
	rest = url+strlen((*info).protocol)+3;
    char* res;
    res = strstr(rest, "/");
	if(res == NULL) exit_with_error("the url is not valid!");
	if(strlen(res) == 1) (*info).path = "";
	else strcpy((*info).path, res+1);
	res = strstr(rest, ":");
	char* h;
	if(res == NULL){
		h = strtok(NULL, "/");
		(*info).host = strtok(h, "/");
		(*info).port = 80;
	}
	else{
		h = strtok(NULL, ":");
		(*info).port = atoi(strtok(NULL, "/"));
		(*info).host = strtok(h, "/");
	}

}

/**
 * print the url info to std output
 */
void print_url_info(url_info info){
	printf("The URL contains following information: \n");
	printf("Protocol type:\t%s\n", info.protocol);
	printf("Host name:\t%s\n", info.host);
	printf("Port No.:\t%d\n", info.port);
	printf("Path:\t\t%s\n", info.path);
}

/**
 * exit with an error message
 */

void exit_with_error(char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}
