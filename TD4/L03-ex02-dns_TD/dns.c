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
	dns->qr = htons(query);
	dns->qd_count = htons(qd_count);
	dns->an_count = htons(an_count);
	dns->ns_count = htons(ns_count);
	dns->ar_count = htons(ar_count);
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
	build_dns_header(dns,23,0,1,0,0,0);
	uint8_t *qname;
	qname = (uint8_t*)&buf[sizeof(dns_header)];
	int position =  0;
	build_name_section(qname, host_name, &position);
	//printf("after build: %s\n", qname);
	struct question* ques;
	ques = (question*)(qname + position+1);
	ques->qtype = htons(TYPE_A);
	ques->qclass = htons(CLASS_IN);
	
	// for(int i=0; i<(sizeof(dns_header)+position+1+sizeof(question)); i++){
	// 	printf("%d \n", buf[i]);
	// }
	if(sendto(sockfd,buf,sizeof(dns_header)+position+1+sizeof(question),0,server,sizeof(struct sockaddr))<0){
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
    printf("\n %d Questions.",ntohs(dns->qd_count));
    printf("\n %d Answers.",ntohs(dns->an_count));
    printf("\n %d Authoritative Servers.",ntohs(dns->ns_count));
    printf("\n %d Additional records.\n\n",ntohs(dns->ar_count));

    //read the query
	uint8_t* reader = &buf[sizeof(dns_header)];

	//printf("query in parse %s\n", (char *) reader);

	// queries->qname = (uint8_t*)reader;
	// printf("query in parse: %s\n",(char *) queries->qname);
	// reader += (strlen((char*)queries->qname)+1);
	// queries->ques = (question*)reader;

	printf("Queries:\n");
	uint8_t qname[HOST_NAME_SIZE];
	int position = 0;
	get_domain_name(reader, buf, qname, &position);
	queries->qname = malloc(HOST_NAME_SIZE);
	memset(queries->qname, 0, HOST_NAME_SIZE);
	strncpy((char*)(queries->qname), (char*)qname, strlen((char*)qname));
	printf("name: %s \n", queries->qname);
	reader += position;
	reader++;

	queries->ques = (question*)reader;
	printf("query type: %d, class: %d\n",
			ntohs(queries->ques->qtype), ntohs(queries->ques->qclass));
	//reader += sizeof(question);


	//read the answers
	reader += sizeof(question);
	printf("\nAnswers:\n");
	//position = 0;
	for(int i=0; i<ntohs(dns->an_count); i++){
		printf("Answer %d\n", i+1);

		uint8_t name[HOST_NAME_SIZE];
		//int position = 0;
		get_domain_name(reader, buf, name, &position);
		//answers[i].name = calloc(1, HOST_NAME_SIZE);
		
		answers[i].name = (uint8_t*)malloc(sizeof(uint8_t)*HOST_NAME_SIZE);
		strncpy((char*)(answers[i].name), (char*)name, strlen((char*)name));
		printf("name: %s \n", answers[i].name);
		//get_domain_name(reader,buf,answers[i].name,&position);
		reader += position;
		//reader ++;

		//answers[i].element = (r_element*)malloc(10);
		answers[i].element = (r_element*)reader;
		int length = ntohs(answers[i].element->rdlength);
		printf("type: %d, class: %d, ttl: %d, rdlength: %d\n",
				ntohs(answers[i].element->type), ntohs(answers[i].element->_class),
				ntohl(answers[i].element->ttl), length);

		reader += sizeof(r_element);
		if(ntohs(answers[i].element->type) == TYPE_A) //ipv4 address
		{
			answers[i].rdata = (uint8_t *)malloc(length);
			memset(answers[i].rdata, 0, length);
			memcpy(answers[i].rdata, reader, length);

			char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
			inet_ntop(AF_INET, answers[i].rdata, ip4, INET_ADDRSTRLEN);
			printf("The IPv4 address is: %s\n", ip4);

		}

		reader += ntohs(answers[i].element->rdlength);
	}

	//read the authorities
	for(int i=0; i<dns->ns_count; i++){
		// get_domain_name(reader,buf,auth[i].name,&position);
		// reader += position;
		// auth[i].element = (r_element*)reader;
		// reader += 10;
		// auth[i].rdata = (uint8_t*)reader;
		// reader += (strlen((char*)auth[i].rdata)+1);
	}

	//read the additionnals
	for(int i=0; i<dns->ar_count; i++){
		// get_domain_name(reader,buf,addit[i].name,&position);
		// reader += position;
		// addit[i].element = (r_element*)reader;
		// reader += 10;
		// addit[i].rdata = (uint8_t*)reader;
		// reader += (strlen((char*)addit[i].rdata)+1);
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

// void get_dns_name(uint8_t *dns, uint8_t *host)
// {
//     //printf("host : %s\n", (char *)host);

//     char host_cp[HOST_NAME_SIZE];
//     strncpy(host_cp, (char*)host, HOST_NAME_SIZE);

// //  printf("host name: %s\n", host_cp);

//     char num[3];
//     char *tk;
//     tk = strtok(host_cp, ".");
//     int i = 0;
//     while(tk!=NULL)
//     {
//         sprintf(num, "%d", (int)strlen(tk));

//         //printf( "%lu\n", strlen(tk));
//         //*(dns+i) = (uint8_t)(strlen(tk)); //set the number of chars in the label
//         *(dns+i) = (uint8_t)(strlen(num)+strlen(tk)); //set the number of chars in the label

//         //i++;
//         strncpy((char*)(dns+i), num, strlen(num)); //the label
//         i+= strlen(num);
//         strncpy((char*)(dns+i), tk, strlen(tk)); //the label
//         //printf("%s\n", (dns+i));
//         i+= strlen(tk);
//         //i+= strlen(num);
//         tk = strtok(NULL,".");
//     }
//     *(dns+i) = '\0';

//     printf("dns : %s\n", (char *)dns);
// }

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

