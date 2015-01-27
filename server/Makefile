CC = g++
CFLAGS  = -g -Wall

default: httpd

httpd:	main.o server.o
	$(CC) $(CFLAGS) -o httpd main.o server.o

main.o:	main.cpp
	$(CC) $(CFLAGS) -c main.cpp

server.o:	server.cpp server.h
	$(CC) $(CFLAGS) -c server.cpp

clean: 
	$(RM) httpd *.o *~
