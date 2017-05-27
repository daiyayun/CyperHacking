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
	struct sockaddr_in *client = (struct sockaddr_in*)(&clientaddr);

	int port = 5000; //the default port of DNS service
	uint8_t buf[BUF_SIZE], send_buf[BUF_SIZE];

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
	res_record answers[10], auth[10], addit[10];
	query queries[10];

	while(1){
		recvfrom(sockfd, buf, BUF_SIZE-1, 0, &clientaddr, &clientlen);
		printf("\nReceived data from %s at port %d\n",
				inet_ntoa(client->sin_addr), ntohs(client->sin_port));

		
		int id = parse_dns_query(buf, queries, answers, auth, addit);

		dns_header *dns = (dns_header*)&send_buf;
		int count = 0;
		build_dns_header(dns, id, 1, 1, 1, 0, 0);

		//we construct 1 question section
		uint8_t *qname = (uint8_t*)&send_buf[sizeof(dns_header)];
		count += sizeof(dns_header);
		int offset = 0;

		build_name_section(qname, (char*)(queries->qname), &offset);
		count += offset;
		count ++;

		question *qdata = (question*)(qname + offset + 1);

		qdata->qtype = htons(TYPE_A);
		qdata->qclass = htons(CLASS_IN);
		count += 4;
		qname = (uint8_t*)qdata + 4;
		
		//answer section
		build_name_section(qname, (char*)queries->qname, &offset);
		count += offset;

		r_element *ele = (r_element*)(qname + offset);
		ele->type = htons(TYPE_A);
		ele->_class = htons(CLASS_IN);
		ele->ttl = htonl(255);
		ele->rdlength = htons(4);
		count += 10; 

		uint8_t *addr = (uint8_t*)(ele) + 10;

		char *IP = "216.58.201.228"; 

		inet_pton(AF_INET, IP, addr);
		count += IN_SIZE;

		if(sendto(sockfd,send_buf,count,0,&clientaddr,clientlen)<0){
			exit_with_error("sendto() failed.");
		}
		memset(buf, 0, BUF_SIZE);
		memset(send_buf, 0, BUF_SIZE);
	}
}
