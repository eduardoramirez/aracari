CC = g++
CFLAGS  = -g -std=c++11 -Wall

default: httpd

httpd:	main.o server.o http.o lib.o htaccess.o
	$(CC) $(CFLAGS) -o httpd main.o http.o server.o lib.o htaccess.o

main.o:	main.cpp
	$(CC) $(CFLAGS) -c main.cpp

server.o:	server.cpp server.h
	$(CC) $(CFLAGS) -c server.cpp

http.o:	http.cpp http.h
	$(CC) $(CFLAGS) -c http.cpp

lib.o: lib.cpp lib.h
	$(CC) $(CFLAGS) -c lib.cpp

htaccess.o: htaccess.cpp htaccess.h
	$(CC) $(CFLAGS) -c htaccess.cpp

clean: 
	$(RM) httpd *.o *~
