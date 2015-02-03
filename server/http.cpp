#include "http.h"

HttpRequest::HttpRequest(string request, int csock, Server * server) {
  fprintf(stderr, "Request received. Printing it out.\n");
  fprintf(stderr, "%s\n", request.c_str());

  this->request = request;
  this->server = server;

  this->docroot = server->getDocroot();

  response = "";
  path = "";
  contentType = "";
  httpType = "";

  beenParsed = false;
  isMalformed = false;
  accessDenied = false;
  notFound = false;
  responseGenerated = false;
  persistent = false;

  this->csock = csock;
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

  travel = travel + tokenLength;
  
  if(travel[0] != '\r' || travel[1] != '\n') {
    isMalformed = true;
    return;
  }

  travel = travel + 2;

  while(travel[0] != '\r' || travel[1] != '\n') {
    if(travel[0] == ':'  ||
       travel[0] == ' '  ||
       travel[0] == '\t' ||
       travel[0] == '\r' ||
       travel[0] == '\n') {
      isMalformed = true;
      return;
    }

    travel = skipTillColon(travel);
    travel = skipTrim(travel);

    if(travel[0] == '\r' ||
       travel[0] == '\n') {
      isMalformed = true;
      return;
    }
    travel++;
    travel = skipTillCRLF(travel);
  }
}


void HttpRequest::generateResponse() {

  if(isMalformed) {
    string temp = request400().c_str();
    send(csock, temp.c_str(), temp.length(), 0);
    close(csock);
    exit(1);
  }

  if(accessDenied) {
    string temp = request403().c_str();
    send(csock, temp.c_str(), temp.length(), 0);
    close(csock);
    exit(1);
  }

  if(notFound) {
    string temp = request404().c_str();
    send(csock, temp.c_str(), temp.length(), 0);
    return;
  }

  request200();

  if(!persistent) {
    close(csock);
    exit(1);
  }
}


char * HttpRequest::skipTrim(char * arr) {
  while(*arr == ' ' || *arr == '\t') {
    arr++;
  }

  return arr;
}


char * HttpRequest::skipTillCRLF(char * arr) {
  while(arr[0] != '\r' || arr[1] != '\n') {
    arr++;
  }
  arr += 2;
  return arr;
}

char * HttpRequest::skipTillColon(char * arr) {
  while(arr[0] != ':') {
    arr++;
  }
  arr += 1;
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
  // TODO Add checks
  // Check if permissions
  // Check if found
  // Check if directory
  // Check file extension (to get content type)
  // Check if directory
  // Set content type


  string remaining(arr, length);

  path = docroot + remaining;

  char buf[BUFSIZ];
  realpath(path.c_str(), buf);
  path = buf;

  const char * dpath = docroot.c_str();

  for(int i = 0; i < docroot.length(); i++) {
    if(dpath[i] != buf[i]) {
      accessDenied = true;
      return false;
    }
  }

  return true;
}

void HttpRequest::request200() {
  string ret = httpType + " 200 Good Request\r\n";

  // TODO Change content type based on extension of path
  //ret += "Content-Type: text/html\r\n";
  ret += "Content-Type: " + contentType + "\r\n";

  FILE * file;

  if((file = fopen(path.c_str(), "rb")) == NULL) {
    fprintf(stderr, "Couldn't open file\n");
    cerr << "Path: " << path << endl;
    close(csock);
    exit(1);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  fseek(file, 0L, SEEK_SET);

  ret += "Content-Length: " + to_string(fileSize) + "\r\n\r\n";

  send(csock, ret.c_str(), ret.length(), 0);

  size_t readSize;

  unsigned char buf[BUFSIZ];

  while(!feof(file)) {
    readSize = fread(buf, 1, BUFSIZ-1, file);

    send(csock, buf, readSize, 0);
  }

  // TODO Don't forget EXIT/PERSISTENCE
}

string HttpRequest::request400() {
  string ret = httpType + " 400 Bad Request\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 13\r\n\r\n";
  ret += "Bad Request!\n";
  return ret;
}

string HttpRequest::request403() {
  string ret = httpType + " 403 Forbidden\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 11\r\n\r\n";
  ret += "Forbidden!\n";
  return ret;
}

string HttpRequest::request404() {
  string ret = httpType + " 404 Not Found\r\n";
  ret += "Content-Type: text/html\r\n";
  ret += "Content-Length: 11\r\n\r\n";
  ret += "Not Found!\n";
  return ret;
}

bool HttpRequest::isPersistent() {
  return persistent;
}

bool HttpRequest::checkHttp(char * arr) {
  if (!((arr[0] == 'H' || arr[0] == 'h') &&
        (arr[1] == 'T' || arr[1] == 't') &&
        (arr[2] == 'T' || arr[2] == 't') &&
        (arr[3] == 'P' || arr[3] == 'p') &&
        (arr[4] == '/') &&
        (arr[5] == '1') &&
        (arr[6] == '.') &&
        (arr[7] == '1' || arr[7] == '0'))) {
    return false;
  }

  if(arr[7] == '1') {
    persistent = true;
    httpType = "HTTP/1.1";
  }
  else {
    persistent = false;
    httpType = "HTTP/1.0";
  }

  return true;
}

bool HttpRequest::checkGet(char * arr) {
  if(!((arr[0] == 'G' || arr[0] == 'g') &&
       (arr[1] == 'E' || arr[1] == 'e') &&
       (arr[2] == 'T' || arr[2] == 't'))) {
    isMalformed = true;
    return false;
  }
  return true;
}
