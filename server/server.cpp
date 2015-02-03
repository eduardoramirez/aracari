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

  // These booleans deal with poor design
  bool breaker = false;
  bool continuer = false;
  bool once = false;

  string extra = "";

  while(true) {
    if(extra.length() != 0) {
      char * extraArr = (char *) extra.c_str();

      for(int i = 0; i < extra.length()-3; i++) {
        if(checkCRLF(extraArr + i)) {
          string newRequest(extraArr, i+4);
          
          HttpRequest httpRequest(newRequest, csock, this);
          httpRequest.parseRequest();
          httpRequest.generateResponse();

          string extraRemains(extraArr, extra.length() - (i+4));
          extra = extraRemains;

          continuer = true;
          break;
        }
      }

      if(continuer) {
        continue;
      }
    }

    fprintf(stderr, "We got here\n");

    string request = extra;
    extra = "";
    breaker = false;
    continuer = false;
    
    char buffer[BUFSIZ];

    char * buf = buffer;

    char last3[3];
    copyLast3(last3,(char *)request.c_str(),request.length());

    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(csock, &rfds);

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    int retval = select(csock+1, &rfds, NULL, NULL, &tv);

    if (retval == -1) {
      perror("select()");
      exit(1);
    }
    else if(retval) {
    }
    else {
      close(csock);
      exit(1);
    }

    while(true) {
      once = true;
      ssize_t bytes_read = recv(csock, buf, BUFSIZ - 1, 0);

      if(bytes_read < 0) {
        continue;
      }

      if(bytes_read == 0) {
        close(csock);
        exit(0);
      }

      int lastIndex;

      if((lastIndex = checkLast3(last3, buf, bytes_read)) >= 0) {
        string remaining(buf, lastIndex+1);
        request = request + remaining;

        HttpRequest httpRequest(request, csock, this);
        httpRequest.parseRequest();
        httpRequest.generateResponse();

        request = "";
        resetLast3(last3);

        buf = buf + lastIndex + 1;
        string extraRemains(buf, bytes_read - (lastIndex+1));
        extra = extraRemains;
        
        breaker = true;
      }

      if(breaker) {
        break;
      }

      for(int i = 0; i < bytes_read - 3; i++) {
        if(checkCRLF(buf+i)) {
          string remaining(buf, i+4);
          request = request + remaining;

          HttpRequest httpRequest(request, csock, this);
          httpRequest.parseRequest();
          httpRequest.generateResponse();

          buf = buf + i + 4;;
          string extraRemains(buf, bytes_read - (i+4));
          extra = extraRemains;

          breaker = true;
        }

        if(breaker) {
          break;
        }
      }

      if(breaker) {
        break;
      }

      resetLast3(last3);
      copyLast3(last3,(char *)request.c_str(),request.length(),buf,bytes_read);

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

void Server::copyLast3(char * arr, char * buf, int length) {
  for(int i = length - 1, j = 2; j >= 0 && i >= 0; i--, j--) {
    arr[j] = buf[i];
  }
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

  for(int i = length - 1, j = 2; j >= 0 && i >= 0; i--, j--) {
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
