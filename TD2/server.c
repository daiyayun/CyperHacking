/*
 *server.c - A simple UDP ech server
 *usage: server <port>
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

/*
 *exit_with_error - wrapper for perror
 */
void exit_with_error(char* msg){
	perror(msg);
	exit(1);
}

int main(int argc, char* argv[]){
	int sockfd;
	int port;
	unsigned int clientlen;//byte size of client's address
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	char buf[BUFSIZE];
	int n;//byte size of received message

	if(argc != 2){
		fprintf(stderr, "The program needs exactly one parameter.\n");
		exit(1);
	}
	port = atoi(argv[1]);

	//create a socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
		exit_with_error("ERROR opening socket");
	printf("Socket created\n");

	memset(&serveraddr, '0', sizeof(serveraddr));
	memset(buf, '0', BUFSIZE);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);

	//bind: associate the socket with the port
	if(bind(sockfd, (struct sockaddr*)&serveraddr, 
			sizeof(serveraddr)) < 0)
		exit_with_error("ERROR on binding");

	//main loop: wait for a datagram, then echo it
	clientlen = sizeof(clientaddr);
	while(1){
		//receive a UDP datagram from a client
		n = recvfrom(sockfd, buf, BUFSIZE-1, 0,
					(struct sockaddr*)&clientaddr, &clientlen);
		if(n < 0)
			exit_with_error("ERROR in recvfrom");
		buf[n] = '\0';
		//sendto: echo the datagram back to the client
		if(sendto(sockfd, buf, strlen(buf), 0,
					(struct sockaddr*)&clientaddr, clientlen) < 0)
			exit_with_error("ERROR in sendto");
	}
	return 0;
}