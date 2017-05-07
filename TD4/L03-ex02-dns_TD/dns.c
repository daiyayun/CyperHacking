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
	dns->id = (uint16_t)id;
	dns->qr = query;
	dns->qd_count = qd_count;
	dns->an_count = an_count;
	dns->ns_count = ns_count;
	dns->ar_count = ar_count;
}

void build_name_section(uint8_t *qname, char *host_name, int *position)
{
	get_dns_name(qname, (uint8_t*)host_name);
	*position = strlen((char*)qname);
}



void send_dns_query(int sockfd, struct sockaddr *server, char *host_name)
{
	uint8_t buf[BUF_SIZE];
	dns_header *dns = NULL;
	dns = (dns_header*)&buf;
	build_dns_header(dns,0,0,1,0,0,0);
	uint8_t *qname;
	qname = (uint8_t*)&buf[sizeof(dns_header)];
	int* position =  (int*)malloc(sizeof(int));
	build_name_section(qname, host_name, position);
	struct question* ques;
	ques = (question*)(qname + (*position)+1);
	ques->qtype = TYPE_TXT;
	ques->qclass = CLASS_IN;
	if(sendto(sockfd,buf,sizeof(dns_header)+(*position)+1+sizeof(question),0,server,sizeof(struct sockaddr))<0){
		exit_with_error("sendto failed");
	}

}

int parse_dns_query(uint8_t *buf, query *queries,
		res_record *answers, res_record *auth, res_record *addit)
{
	//read DNS header
	dns_header* dns;
	dns = (dns_header*)buf;
	printf("\nThe response contains : ");
    printf("\n %d Questions.",dns->qd_count);
    printf("\n %d Answers.",dns->an_count);
    printf("\n %d Authoritative Servers.",dns->ns_count);
    printf("\n %d Additional records.\n\n",dns->ar_count);

    //read the query
	uint8_t* reader = &buf[sizeof(dns_header)];
	queries->qname = (uint8_t*)reader;
	reader += (strlen((char*)queries->qname)+1);
	queries->ques = (question*)reader;

	//read the answers
	reader += sizeof(question);
	int position = 0;
	for(int i=0; i<dns->an_count; i++){
		get_domain_name(reader,reader,answers[i].name,&position);
		reader += position;
		answers[i].element = (r_element*)reader;
		reader += 10;
		answers[i].rdata = (uint8_t*)reader;
		reader += (strlen((char*)answers[i].rdata)+1);
	}

	//read the authorities
	for(int i=0; i<dns->ns_count; i++){
		get_domain_name(reader,reader,auth[i].name,&position);
		reader += position;
		auth[i].element = (r_element*)reader;
		reader += 10;
		auth[i].rdata = (uint8_t*)reader;
		reader += (strlen((char*)auth[i].rdata)+1);
	}

	//read the additionnals
	for(int i=0; i<dns->ar_count; i++){
		get_domain_name(reader,reader,addit[i].name,&position);
		reader += position;
		addit[i].element = (r_element*)reader;
		reader += 10;
		addit[i].rdata = (uint8_t*)reader;
		reader += (strlen((char*)addit[i].rdata)+1);
	}

	return dns->id;
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

