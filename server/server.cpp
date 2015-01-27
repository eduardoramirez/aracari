#include "server.h"

Server::Server(int port, string docroot) {
  this->port = port;
  this->docroot = docroot;
}

Server::~Server() {
}

int Server::initialize() {
  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);

  if(bind(sock, (struct sockaddr*) &address, sizeof(address)) < 0) {
    perror("bind failed");
    return 1;
  }

  if(listen(sock, 128) < 0) {
    perror("listen failed");
    return 2;
  }

  return 0;
}

int Server::getSock() {
  return sock;
}

int Server::processRequest(int csock) {
  char buf[BUFSIZ];
  ssize_t bytes_read;

  do {
    bytes_read = recv(csock, &buf, sizeof(buf) - 1, 0);
    if(bytes_read < 0) {
      perror("recv failed");
      exit(1);
    }

    ssize_t bytes_sent = send(csock, &buf, bytes_read, 0);
    if(bytes_sent < 0) {
      perror("send failed");
      exit(1);
    } else if (bytes_sent < bytes_read) {
      perror("couldn't send everything");
      exit(1);
    }

  } while(bytes_read > 0);

  close(csock);
  exit(0);

  return 0;
}
