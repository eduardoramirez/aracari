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

  if(tokenLength != 3) {
    isMalformed = true;
    return;
  }
  
  if(travel[0] != 'G' || travel[1] != 'E' || travel[2] != 'T') {
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
    return;
  }
  
}


void HttpRequest::generateResponse(int csock) {

  if(isMalformed) {
    send(csock, badRequest().c_str(), badRequest().length(), 0);
    return;
  }
}


char * HttpRequest::skipTrim(char * arr) {
  while(*arr == ' ' || *arr == '\t') {
    arr++;
  }

  return arr;
}

int HttpRequest::getTokenLength(char * arr) {
  char * travel = arr;

  while(*travel != ' ' && *travel != '\t' && *travel != '\r') {
    travel++;
  }

  return (travel-arr);
}

bool HttpRequest::parsePath(char * arr, int length) {
  

  return false;
}

string HttpRequest::badRequest() {
  string ret = "HTTP/1.1 400 Bad Request\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 13\r\n\r\n";
  ret += "Bad Request!\n";
  return ret;
}

string HttpRequest::request404() {
  string ret = "HTTP/1.1 404 Not Found\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 11\r\n\r\n";
  ret += "Not Found!\n";
  return ret;
}

string HttpRequest::goodRequest() {
  string ret = "HTTP/1.1 200 Good Request\r\n";
  return ret;
}
