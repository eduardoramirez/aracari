#include "server.h"



void cleanUp(int sig) {
  fprintf(stderr, "\nCleaning Up\n");
  exit(sig);
}

void usage() {
    fprintf(stderr, "USAGE: ./httpd <port> <document root>\n");
}



int main(int argc, char * argv[]) {
  if(argc != 3) {
    fprintf(stderr, "Incorrect number of arguments\n");
    usage();
    exit(-1);
  }

  signal(SIGINT, cleanUp);

  int port = atoi(argv[1]);

  if(!is_dir(argv[2])) {
    fprintf(stderr, "%s is not a directory\n", argv[2]);
    exit(1);
  }

  string docroot(realpath(argv[2], NULL));

  Server server(port, docroot);

  if(server.initialize() != 0) {
    fprintf(stderr, "Exiting now\n");
    exit(1); 
  }
  
  //fprintf(stderr, "docroot: %s\n", docroot.c_str());

  int sock = server.getSock();

  while(true) {
    struct sockaddr_in client_address = {0};
    socklen_t ca_len = 0;

    int csock = accept(sock, (struct sockaddr*) &client_address, &ca_len);

    if(csock < 0) {
      perror("accept failed");
      // TODO continue or exit?
      continue;
    }

    if(fork() == 0) {
      server.processRequest(csock);
    }
  }

  close(sock);
  exit(0);
}
