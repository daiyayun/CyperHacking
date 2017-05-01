/*
 * dns.c
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


void build_dns_header(dns_header *dns, int id, int query, int qd_count,
		int an_count, int ns_count, int ar_count)
{
}

void build_name_section(uint8_t *qname, char *host_name, int *position)
{
}



void send_dns_query(int sockfd, char *dns_server, char *host_name)
{
}

int parse_dns_query(uint8_t *buf, query *queries,
		res_record *answers, res_record *auth, res_record *addit)
{
}


void get_domain_name(uint8_t *p, uint8_t *buff, uint8_t *name, int *position)
{
	uint8_t first = *p;
	bool compressed = false;

	if( (first & 0xc0) == 0xc0) //compression. move the pointer to the real location
	{
		//	The pointer takes the form of a two octet sequence:
		//
		//	    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		//	    | 1  1|                OFFSET                   |
		//	    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		//
		//	The first two bits are ones. The OFFSET field specifies an offset from
		//	the start of the message (i.e., the first octet of the ID field in the
		//	domain header).

		//		printf("compression \n");
		uint16_t offset = ntohs(*((uint16_t*)p)) & 0x3fff;
		//		printf("the offfset is %d\n", offset);

		p = buff+offset;
		compressed = true;
		*position = 2; //move ahead 2 octets
		//		printf("The real name is %s \n", (char*)p);
	}

	int i = 0;
	while(*p!=0)
	{
		uint8_t num = *((uint8_t*)p);
		strncpy((char*)(name+i), (char*)(p+1), num);
		p+= (num+1);
		i+= num;
		strncpy((char*)(name+i), ".", 1);
		i++;
		//		printf("the count is: %d\n", num);
	}
	*(name+i)='\0';

	if(compressed == false) //if not compressed
	{
		*position = i;
	}
	//	printf("The generated name is %s\n", name);
}

void get_dns_name(uint8_t *dns, uint8_t *host)
{
	char host_cp[HOST_NAME_SIZE];
	strncpy(host_cp, (char*)host, HOST_NAME_SIZE);

//	printf("host name: %s\n", host_cp);

	char *tk;
	tk = strtok(host_cp, ".");
	int i = 0;
	while(tk!=NULL)
	{
		//		sprintf(length, "%lu", strlen(tk));
		*(dns+i) = (uint8_t)(strlen(tk)); //set the number of chars in the label

		i++;
		strncpy((char*)(dns+i), tk, strlen(tk)); //the label

		i+= strlen(tk);
		tk = strtok(NULL,".");
	}
	*(dns+i) = '\0';
}

/**
 * exit with an error message
 */

void exit_with_error(char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}

