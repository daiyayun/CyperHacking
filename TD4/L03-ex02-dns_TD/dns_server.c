/*
 * dns_server.c
 *
 *  Created on: Apr 26, 2016
 *      Author: jiaziyi
 */


#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdbool.h>
#include<time.h>

#include "dns.h"

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr server;

	int port = 53; //the default port of DNS service
	uint8_t buf[BUF_SIZE];
	if(recvfrom())
}
