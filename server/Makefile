CC = g++
CFLAGS  = -g -Wall

default: httpd

httpd:	server.o
	$(CC) $(CFLAGS) -o httpd server.o

server.o:	server.cpp server.h
	$(CC) $(CFLAGS) -c server.cpp

clean: 
	$(RM) httpd *.o *~
