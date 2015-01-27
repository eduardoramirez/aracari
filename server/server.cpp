#include <stdio.h>
#include <stdlib.h>
#include "server.h"

int main(int argc, char * argv[]) {
  if(argc != 3) {
    fprintf(stderr, "USAGE: ./httpd <port> <document root>\n");
  }
}
