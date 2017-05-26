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


	//to keep the information received.
	res_record answers[ANS_SIZE], auth[ANS_SIZE], addit[ANS_SIZE];
	query queries[ANS_SIZE];


	if(argc == 2)
	{
		port = atoi(argv[1]); //if we need to define the DNS to a specific port
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	int enable = 1;

	if(sockfd <0 )
	{
		perror("socket creation error");
		exit_with_error("Socket creation failed");
	}

	//in some operating systems, you probably need to set the REUSEADDR
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
	    perror("setsockopt(SO_REUSEADDR) failed");
	}

	//for v4 address
	struct sockaddr_in *server_v4 = (struct sockaddr_in*)(&server);
	server_v4->sin_family = AF_INET;
	server_v4->sin_addr.s_addr = htonl(INADDR_ANY);
	server_v4->sin_port = htons(port);

	//bind the socket
	if(bind(sockfd, &server, sizeof(*server_v4))<0){
		perror("Binding error");
		exit_with_error("Socket binding failed");
	}

	printf("The dns_server is now listening on port %d ... \n", port);
	//print out
	uint8_t buf[BUF_SIZE], send_buf[BUF_SIZE]; //receiving buffer and sending buffer
	struct sockaddr remote;
	int n;
	socklen_t addr_len = sizeof(remote);
	struct sockaddr_in *remote_v4 = (struct sockaddr_in*)(&remote);

	while(1)
	{
		//an infinite loop that keeps receiving DNS queries and send back a reply
		//complete your code here
		//BEGIN_SOLUTION
		//receive the query
		n = recvfrom(sockfd, buf, BUF_SIZE, 0, &remote, &addr_len);
		printf("\n\nDatagram received from %s at port %d\n",
				inet_ntoa(remote_v4->sin_addr), ntohs(remote_v4->sin_port));

		if(n<0)
		{
			perror("Error receiving UDP data");
		}

		//parse the dns query
		int id = parse_dns_query(buf, queries, answers, auth, addit);

		//construct response

		//header
		dns_header *dns = (dns_header*)&send_buf;
		int size = 0;
		build_dns_header(dns, id, 1, 1, 1, 0, 0);

		//we construct 1 question section
		uint8_t *qname = (uint8_t*)&send_buf[sizeof(dns_header)];
		size += sizeof(dns_header);
		int offset = 0;

		for(int i=0;i<ANS_SIZE;i++)
		{
			if(queries[i].qname == NULL)
				break;

			build_name_section(qname, (char*)(queries[i].qname), &offset);
			size += offset;

			question *qdata = (question*)(qname + offset);

			qdata->qtype = htons(TYPE_A);
			qdata->qclass = htons(CLASS_IN);
			size += 4;
			qname = (uint8_t*)qdata + 4;
		}

		//1 answer section

		//host name
		build_name_section(qname, (char*)queries[0].qname, &offset); //the first query
//		get_dns_name(qname, (uint8_t*)host_name);
//		*position = strlen((char*)qname) + 1; //calculate the offset

		size += offset;

		//fixed size section
		r_element *r = (r_element*)(qname + offset);
		r->type = htons(TYPE_A);
		r->_class = htons(CLASS_IN);
		r->ttl = htonl(255);
		r->rdlength = htons(4);
		size += 10; //attention: we can't use sizeof(r_element) !!!

		uint8_t *addr = (uint8_t*)(r) + 10;

		//address

		char *address_array = "192.168.1.102"; //the answer to put...

		inet_pton(AF_INET, address_array, addr);
		size += IN_SIZE;

		//send the answer back
		int s = sendto(sockfd, send_buf, size, 0, &remote, addr_len);
		if(s<0)
		{
			perror("Error replying the DNS query. ");
		}
		memset(buf, 0, BUF_SIZE);
		memset(send_buf, 0, BUF_SIZE);
		//END_SOLUTION
	}
}
