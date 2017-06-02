/*
 * rawip_example.c
 *
 *  Created on: May 4, 2016
 *      Author: jiaziyi
 */


#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include "header.h"

//#define SRC_IP  "192.168.1.111" //set your source ip here. It can be a fake one
#define SRC_IP  "10.200.0.124" //set your source ip here. It can be a fake one
#define SRC_PORT 54321 //set the source port here. It can be a fake one

#define DEST_IP "192.168.1.123" //set your destination ip here
#define DEST_PORT 5555 //set the destination port here
#define TEST_STRING "test data" //a test string as packet payload

int main(int argc, char *argv[])
{
	//char source_ip[] = SRC_IP;
	//char dest_ip[] = DEST_IP;


	int fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    int hincl = 1;                  /* 1 = on, 0 = off */
    setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl));

	if(fd < 0)
	{
		perror("Error creating raw socket ");
		exit(1);
	}

	char packet[65536], *data;
	char data_string[] = TEST_STRING;
	char src_ip[] = SRC_IP;
	char dst_ip[] = DEST_IP;
	memset(packet, 0, 65536);

	//IP header pointer
	struct iphdr *iph = (struct iphdr *)packet;

	//UDP header pointer
	struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct iphdr));
	//struct pseudo_udp_header psh; //pseudo header

	//data section pointer
	data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

	//fill the data section
	strncpy(data, data_string, strlen(data_string));

	//fill the IP header here
	int payload = strlen(data_string);
	iph->version = 4;
	iph->ihl = 5;
	iph->tos = 0;
	//iph->tot_len=htons(payload + sizeof(struct iphdr) + sizeof(struct udphdr));
	iph->tot_len=payload + sizeof(struct iphdr) + sizeof(struct udphdr);
	iph->id = htons(0);
	iph->frag_off = 0;
	iph->ttl = 255; //time to live is eight
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;
	iph->check = checksum((unsigned short*)iph, sizeof(struct iphdr));

	int status;
	// Source IPv4 address (32 bits)
    if ((status = inet_pton (AF_INET, src_ip, &(iph->saddr))) != 1) {
        fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    	exit (EXIT_FAILURE);
    }

    // Destination IPv4 address (32 bits)
    if ((status = inet_pton (AF_INET, dst_ip, &(iph->daddr))) != 1) {
    	fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    	exit (EXIT_FAILURE);
  	}

  	printf("ip filled\n");
	//fill the UDP header
	udph->source = htons(SRC_PORT);
	udph->dest = htons(DEST_PORT);
	udph->len = htons(sizeof(struct udphdr)+payload);
	udph->check = 0;
	udph->check = udp4_checksum (*iph, *udph, (uint8_t *)data_string, payload);
	printf("udp filled\n");
	

	//send the packet
	struct sockaddr_in dest;
	memset(&dest, '0', sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = iph->daddr;
	dest.sin_port = htons(DEST_PORT);
	if (sendto (fd, packet, payload + sizeof(struct iphdr) + sizeof(struct udphdr), 0, (struct sockaddr *) &dest, sizeof (struct sockaddr)) < 0)  {
    	perror ("sendto() failed ");
    	exit (EXIT_FAILURE);
  	}

	return 0;

}
