all: fancyclient client server

fancyclient: fancyclient.o
	cc -g -o fancyclient fancyclient.o

client: client.o
	cc -g -o client client.o

server: server.o
	cc -g -o server server.o

%.o : %.c
		cc -g -Wall -o $@ -c $<

clean: 
		rm -f fancyclient client server \
			fancyclient.o client.o server.o
