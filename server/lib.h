#ifndef LIB
#define LIB

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

using namespace std;

bool is_file(char * path);
bool is_dir(char * path);

#endif
