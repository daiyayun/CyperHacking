#ifndef TEST_URL_H
#define TEST_URL_H

struct url_info {
	char* protocol;
	char* hostame;
	int port;
	char* path;
};

typedef struct url_info url_info;

url_info parseURL(char* url);
bool validateURL(url_info info);

#endif