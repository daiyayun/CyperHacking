/*
 * dns_server.c
 *
 *  Created on: Apr 26, 2016
 *      Author: jiaziyi
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <time.h>
#include <netdb.h>

#include "dns.h"

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in serveraddr;
	struct sockaddr clientaddr;

	int port = 5000; //the default port of DNS service
	uint8_t buf[BUF_SIZE];

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset(&serveraddr, '0', sizeof(serveraddr));
	memset(buf, '0', BUF_SIZE);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);

	//bind: associate the socket with the port
	if(bind(sockfd, (struct sockaddr*)&serveraddr, 
			sizeof(serveraddr)) < 0)
		exit_with_error("ERROR on binding");
	
	//receive a query from the client
	unsigned int clientlen = sizeof(clientaddr);
	while(1){
		recvfrom(sockfd, buf, BUF_SIZE-1, 0, &clientaddr, &clientlen);
		res_record answers[10], auth[10], addit[10];
		query queries[10];
		parse_dns_query(buf, queries, answers, auth, addit);
		uint8_t name[HOST_NAME_SIZE];
		int position=0;
		get_domain_name(queries->qname,queries->qname,name,&position);
		struct hostent* host;
		host = gethostbyname((char*)name);
		printf("hostname requested: %s\n", host->h_name);
		uint8_t* rname;

		dns_header *dns = NULL;
		dns = (dns_header*)&buf;
		dns->qr = htons(1);
		dns->an_count = htons(1);

		rname = &buf[sizeof(dns_header)+position+1+sizeof(question)];
		strcpy((char*)rname, host->h_name);
		r_element *ele;
		ele = (r_element*)(rname+strlen((char*)rname)+1);
		ele->type = htons(TYPE_A);
		ele->_class = htons(CLASS_IN);
		ele->ttl = 183;
		ele->rdlength = 4;
		uint8_t* rdata;
		rdata = (uint8_t*)(ele + 10);
		char* ip = (char*)malloc(16);
		inet_ntop(AF_INET, (void *)host->h_addr_list[0], ip, 15);
		inet_pton(AF_INET, ip, (void *)&rdata);

		if(sendto(sockfd,buf,sizeof(dns_header)+position+1+sizeof(question)+strlen((char*)rname+15),
			0,&clientaddr,clientlen)<0){
			exit_with_error("sendto failed");
		}
	}
}
