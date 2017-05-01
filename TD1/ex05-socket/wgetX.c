
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>


#include "url.h"
#include "wgetX.h"

int main(int argc, char* argv[])
{

	url_info info;


	if (argc != 2) {
		exit_with_error("The wgetX must have exactly 1 parameter as input. \n");
	}
	char *url = argv[1];

	printf("Downloading %s \n", url);

	//get the url
	parse_url(url, &info);

	print_url_info(info);

	//download page
	char *recv_buf_t;
	recv_buf_t = malloc(sizeof(char)*B_SIZE);
	bzero(recv_buf_t, B_SIZE);//memset? sizeof(recv_buf_t) is not wrong?
	char *buff = download_page(info, recv_buf_t);
	puts(buff);

	//write to the file
	write_data("received_page", buff);

	free(recv_buf_t);

	puts("the file is saved in received_page.");
	return (EXIT_SUCCESS);
}

char* download_page(url_info info, char *recv_buf_t)
{
	int mysocket;
	int tmpres;
	char* get;
	char* ip;
	struct sockaddr_in dest;

	mysocket = socket(AF_INET, SOCK_STREAM, 0);
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	ip = get_ip(info.host);
	tmpres = inet_pton(AF_INET, ip, (void *)(&(dest.sin_addr.s_addr)));
	if( tmpres < 0){
		perror("Can't set dest.sin_addr.s_addr");
		exit(1);
	}else if(tmpres == 0){
		fprintf(stderr, "%s is not a valid IP address\n", ip);
		exit(1);
	}
	dest.sin_port = htons(info.port);

	if(connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) < 0){
    	perror("Could not connect");
    	exit(1);
    }

    get = build_get_query(info.host, info.path);
	fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);

	//Send the query to the server
	int sent = 0;
	while(sent < strlen(get)){
		tmpres = send(mysocket, get+sent, strlen(get)-sent, 0);
		if(tmpres == -1){
		  perror("Can't send query");
		  exit(1);
		}
		sent += tmpres;
	}
	//now it is time to receive the page

	char buf[MAXRCVLEN+1];
	memset(buf, 0, sizeof(buf));
	int htmlstart = 0;
	char * htmlcontent;
	while((tmpres = recv(mysocket, buf, MAXRCVLEN, 0)) > 0){
		if(htmlstart == 0)
		{
		  /* Under certain conditions this will not work.
		  * If the \r\n\r\n part is splitted into two messages
		  * it will fail to detect the beginning of HTML content
		  */

		  htmlcontent = strstr(buf, "\r\n\r\n");
		  if(htmlcontent != NULL){
		    htmlstart = 1;
		    htmlcontent += 4;
		  }
		}else{
		  htmlcontent = buf;
		}
		if(htmlstart){
		  strcat(recv_buf_t, htmlcontent);
		}

		memset(buf, 0, tmpres);
	}
	if(tmpres < 0){
		perror("Error receiving data");
	}

	free(ip);
	free(get);
	close(mysocket);

	return recv_buf_t;
}

/*
	get_ip() and build_get_query are copied from 
		http://coding.debuntu.org/c-linux-socket-programming-tcp-simple-http-client
*/
char *get_ip(char *host)
{
  struct hostent *hent;
  int iplen = 15; //XXX.XXX.XXX.XXX
  char *ip = (char *)malloc(iplen+1);
  memset(ip, 0, iplen+1);
  if((hent = gethostbyname(host)) == NULL)
  {
    herror("Can't get IP");
    exit(1);
  }
  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
  {
    perror("Can't resolve host");
    exit(1);
  }
  return ip;
}

char *build_get_query(char *host, char *path)
{
  char *query;
  char *getpage = path;
  char *tpl = "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\n\r\n";
  if(getpage[0] == '/'){
    getpage = getpage + 1;
    fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", path, getpage);
  }
  // -5 is to consider the %s %s %s in tpl and the ending \0
  query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
  sprintf(query, tpl, getpage, host, USERAGENT);
  return query;
}

void write_data(const char *path, const char * data)
{
	FILE *fp = fopen(path, "w");
	if(fp != NULL){
		fputs(data,fp);
		fclose(fp);
	}
}
