#ifndef LIB
#define LIB

#define INVALID -1

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <string>

#include <sys/stat.h>

#include <iostream>
#include <csignal>

#include <netdb.h>

#include <cstring>

using namespace std;

bool is_file(char * path);
bool is_dir(char * path);

void shut(int csock, int ssock);
void shut(int sock);

#endif
