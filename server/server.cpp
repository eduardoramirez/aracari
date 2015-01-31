#include "server.h"
#include "http.h"

extern char CR;
extern char LF;

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

int Server::getPort() {
  return port;
}

string Server::getDocroot() {
  return docroot;
}

void Server::processRequest(int csock) {

  while(true) {
    string request = "";
    string extra = "";
    
    char buf[BUFSIZ];

    char last3[3];

    while(true) {
      ssize_t bytes_read = recv(csock, &buf, sizeof(buf) - 1, 0);

      if(bytes_read <= 0) {
        continue;
      }

      int lastIndex;

      if((lastIndex = checkLast3(last3, buf, bytes_read)) >= 0) {
        string remaining(buf, lastIndex+1);
        HttpRequest httpRequest(request, this);
      }

      for(int i = 0; i < bytes_read - 3; i++) {
        if(checkCRLF(buf+i)) {
          string remaining(buf, i+4);
          request = request + remaining;

          HttpRequest httpRequest(request, this);
        }
      }

      resetLast3(last3);
      copyLast3(last3, (char *) request.c_str(), request.length(), buf, bytes_read);

      string remaining(buf, bytes_read);
      request = request + remaining;
    }
  }

  close(csock);
  exit(0);


  /*
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
  exit(0);*/
}


bool Server::checkCRLF(char * arr) {
  return (arr[0] == CR &&
          arr[1] == LF &&
          arr[2] == CR &&
          arr[3] == LF);
}

void Server::copyLast3(char * arr, char * buf1, int buf1len, char * buf2, int buf2len) {
  int length = buf1len + buf2len;
  
  char buf[length];

  for(int i = 0; i < buf1len; i++) {
    buf[i] =  buf1[i];
  }

  for(int i = buf1len, j = 0; i < length; i++, j++) {
    buf[i] = buf2[j];
  }

  for(int i = length - 1, j = 2; j >= 0 && length >= 0; i--, j--) {
    arr[j] = buf[i];
  }
}

void Server::resetLast3(char * arr) {
  arr[0] = arr[1] = arr[2] = 0;
}

int Server::checkLast3(char * end, char * start, ssize_t bytes_read) {
  char check[6];

  for(int i = 0; i < 3; i++) {
    check[i] = end[i];

    if(bytes_read > i) {
      check[i+3] = start[i];
    }
  }

  for(int i = 0; i < 3; i++) {
    if(checkCRLF(check+i)) {
      return i;
    }
  }
  return -1;
}
