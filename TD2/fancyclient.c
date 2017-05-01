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

//exit_with_error - wrapper for perror
void exit_with_error(char* msg){
	perror(msg);
	exit(0);
}

int main(int argc, char* argv[]){
	if(argc != 3){
		fprintf(stderr, "The client must have exactly 2 parameters as input.\n");
		exit(0);
	}

	char* IP = argv[1];
	char* PORT = argv[2];
	int sockfd;
	int tmpres;
	struct sockaddr_in dest;
	unsigned int serverlen;

	//create a socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
		exit_with_error("ERROR opening socket");

	memset(&dest, '0', sizeof(dest));
	dest.sin_family = AF_INET;
	tmpres = inet_pton(AF_INET, IP, (void *)(&(dest.sin_addr.s_addr)));
	if( tmpres < 0){
		perror("Can't set dest.sin_addr.s_addr");
		exit(1);
	}else if(tmpres == 0){
		fprintf(stderr, "%s is not a valid IP address\n", IP);
		exit(1);
	}
	dest.sin_port = htons(atoi(PORT));

	//get a message from keyboard
	char message[BUFSIZE];
	memset(message, '0', BUFSIZE);
	printf("Please enter message:\n");
	fgets(message, BUFSIZE-1, stdin);
	message[BUFSIZE] = '\0';

	//send the message to the server
	serverlen = sizeof(struct sockaddr);
	tmpres = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&dest, serverlen);
	if(tmpres == -1)
	  exit_with_error("Can't send query");

	//print the server's reply
	tmpres = recvfrom(sockfd, message, BUFSIZE-1, 0, (struct sockaddr*)&dest, &serverlen);
	if(tmpres == -1)
		exit_with_error("ERROR in recvfrom");
	message[tmpres] = '\0';
	printf("Echo from server:\n%s\n", message);

	close(sockfd);
	return 0;
}