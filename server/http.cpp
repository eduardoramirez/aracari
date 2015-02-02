#include "http.h"

HttpRequest::HttpRequest(string request, Server * server) {
  fprintf(stderr, "Request received. Printing it out.\n");
  fprintf(stderr, "%s\n", request.c_str());

  this->request = request;
  this->server = server;

  this->docroot = server->getDocroot();

  response = "";

  path = "";

  beenParsed = false;
  isMalformed = false;
  accessDenied = false;
  notFound = false;
  responseGenerated = false;

  persistent = false;
}

HttpRequest::~HttpRequest() {
}

void HttpRequest::parseRequest() {
  // First Line GET Location HTTP/1.1
  // Loop for Key value pairs with : separator

  int tokenLength;

  beenParsed = true;

  char * travel = (char *) request.c_str();

  tokenLength = getTokenLength(travel);

  if(tokenLength != 3 || !checkGet(travel)) {
    isMalformed = true;
    return;
  }
  
  travel = travel + tokenLength;
  travel = skipTrim(travel);

  if(travel[0] != '/') {
    isMalformed = true;
    return;
  }

  tokenLength = getTokenLength(travel);

  if(!parsePath(travel, tokenLength)) {
    // TODO Deal with this shit
  }

  travel = travel + tokenLength;
  travel = skipTrim(travel);
  
  tokenLength = getTokenLength(travel);

  if(tokenLength != 8 || !checkHttp(travel)) {
    isMalformed = true;
    return;
  }

}


void HttpRequest::generateResponse(int csock) {

  if(isMalformed) {
    send(csock, request400().c_str(), request400().length(), 0);
    return;
  }

  if(accessDenied) {
    send(csock, request403().c_str(), request403().length(), 0);
    return;
  }

  if(notFound) {
    send(csock, request404().c_str(), request404().length(), 0);
    return;
  }

  send(csock, request200().c_str(), request200().length(),0);
}


char * HttpRequest::skipTrim(char * arr) {
  while(*arr == ' ' || *arr == '\t') {
    arr++;
  }

  return arr;
}

int HttpRequest::getTokenLength(char * arr) {
  char * travel = arr;

  while(*travel != ' ' &&
        *travel != '\t' &&
        *travel != '\r' &&
        *travel != '\n') {
    travel++;
  }

  return (travel-arr);
}

bool HttpRequest::parsePath(char * arr, int length) {
  return false;
}

string HttpRequest::request200() {
  string ret = "HTTP/1.1 200 Good Request\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 14\r\n\r\n";
  ret += "Good Request!\n";
  return ret;
}

string HttpRequest::request400() {
  string ret = "HTTP/1.1 400 Bad Request\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 13\r\n\r\n";
  ret += "Bad Request!\n";
  return ret;
}

string HttpRequest::request403() {
  string ret = "HTTP/1.1 403 Forbidden\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 11\r\n\r\n";
  ret += "Forbidden!\n";
  return ret;
}

string HttpRequest::request404() {
  string ret = "HTTP/1.1 404 Not Found\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 11\r\n\r\n";
  ret += "Not Found!\n";
  return ret;
}

bool HttpRequest::isPersistent() {
  return persistent;
}

bool HttpRequest::checkHttp(char * arr) {
  if (!(arr[0] == 'H' || arr[0] == 'h') &&
      (arr[1] == 'T' || arr[1] == 't') &&
      (arr[2] == 'T' || arr[2] == 't') &&
      (arr[3] == 'p' || arr[3] == 'p') &&
      (arr[4] == '/') &&
      (arr[5] == '1') &&
      (arr[6] == '.') &&
      (arr[7] == '1' || arr[3] == '1')) {
    return false;
  }

  if(arr[7] == '1') {
    persistent = true;
  }
  else {
    persistent = false;
  }

  return true;
}

bool HttpRequest::checkGet(char * arr) {
  if(!(arr[0] == 'G' || arr[0] == 'g') &&
     (arr[1] == 'E' || arr[1] == 'e') &&
     (arr[2] == 'T' || arr[2] == 't')) {
    isMalformed = true;
    return false;
  }
  return true;
}
