#include "http.h"

HttpRequest::HttpRequest(string request, int csock, Server * server) {
  //fprintf(stderr, "Request received. Printing it out.\n");
  //fprintf(stderr, "%s\n", request.c_str());

  this->request = request;
  this->server = server;

  this->docroot = server->getDocroot();
  
  path = "";
  path = path + docroot;
  response = "";
  contentType = "text/html";
  httpType = "HTTP/1.1";

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
    if(isMalformed) {
      return;
    }
    // TODO Maybe more?
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
    shut(csock, server->getSock());
  }

  if(accessDenied) {
    string temp = request403().c_str();
    send(csock, temp.c_str(), temp.length(), 0);
    shut(csock, server->getSock());
  }

  if(notFound) {
    string temp = request404().c_str();
    send(csock, temp.c_str(), temp.length(), 0);
    shut(csock, server->getSock());
    // TODO CLOSE OR NOT???
    return;
  }

  request200();

  if(!persistent) {
    shut(csock, server->getSock());
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
  arr++;
  return arr;
}

char * HttpRequest::skipSlash(char * arr) {
  while(arr[0] == '/') {
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
  // TODO Add checks
  // Check if permissions
  // Check if found
  // Check if directory
  // Check file extension (to get content type)
  // Check if directory
  // Set content type

  if (length >= BUFSIZ/2) {
    fprintf(stderr, "Path name of file you're requesting is too large\n");
    isMalformed = true;
    return false;
  }

  char buf[BUFSIZ];

  for(int i = 0; i < BUFSIZ; i++) {
    buf[i] = 0;
  }

  char * travel = buf;

  for(int i = 0; i < length; i++) {
    travel[i] = arr[i];
  }

  if(travel[length-1] == '/') {
    addIndex(travel+length);
    length += 10;
  }


  int permissionCounter = 0;

  while(travel[0] != '\0') {
    travel = skipSlash(travel);
    int pathTokenLength = getPathTokenLength(travel);
    
    if(pathTokenLength == 2 && travel[0] == '.' && travel[1] == '.') {
      permissionCounter--;
      if(permissionCounter < 0) {
        accessDenied = true;
        return false;
      }
    }
    else {
      permissionCounter++;
    }

    // TODO Now stat for each token

    string token(travel, pathTokenLength);
    path = path + '/' + token;
    
    char tempBuf[BUFSIZ];

    realpath(path.c_str(), tempBuf);

    struct stat statBuf;
    
    if(stat(tempBuf, &statBuf) != 0) {
      perror("WHAT");
      notFound = true;
      fprintf(stderr, "%s\n", tempBuf);
      return false;;
    }
    
    if(!(statBuf.st_mode & S_IROTH)) {
      accessDenied = true;
      return false;
    }

    if(S_ISDIR(statBuf.st_mode)) {
      if(!(statBuf.st_mode & S_IXOTH)) {
        accessDenied = true;
        return false;
      }
    }
    else {
      if(travel[pathTokenLength] == '/') {
        notFound = true;
        return false;
      }
    }    

    travel = travel + pathTokenLength;
  }

  struct stat tempBuf;

  if(is_dir((char *) path.c_str())) {
    path = path + "/index.html";
    
    if(stat(path.c_str(), &tempBuf) != 0) {
      notFound = true;
      return false;
    }
  }


  for(int i = 0; i < BUFSIZ; i++) {
    buf[i] = 0;
  }

  realpath(path.c_str(), buf);
  path = buf;

  setContentType(path);

  const char * dpath = docroot.c_str();

  for(unsigned int i = 0; i < docroot.length(); i++) {
    if(dpath[i] != buf[i]) {
      accessDenied = true;
      return false;
    }
  }

  return true;
}

void HttpRequest::request200() {
  string ret = httpType + " 200 Good Request\r\n";

  ret += "Content-Type: " + contentType + "\r\n";

  FILE * file;

  if((file = fopen(path.c_str(), "rb")) == NULL) {
    fprintf(stderr, "Couldn't open file\n");
    fprintf(stderr, "%s\n", path.c_str());
    shut(csock, server->getSock());
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
}

string HttpRequest::request400() {
  string ret = httpType + " 400 Bad Request\r\n";
  ret += "Content-Type: " + contentType + "\r\n";
  ret += "Content-Length: 13\r\n\r\n";
  ret += "Bad Request!\n";
  return ret;
}

string HttpRequest::request403() {
  string ret = httpType + " 403 Forbidden\r\n";
  ret += "Content-Type: " + contentType + "\r\n";
  ret += "Content-Length: 11\r\n\r\n";
  ret += "Forbidden!\n";
  return ret;
}

string HttpRequest::request404() {
  string ret = httpType + " 404 Not Found\r\n";
  ret += "Content-Type: " + contentType + "\r\n";
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

void HttpRequest::addIndex(char * arr) {
  arr[0] = 'i';
  arr[1] = 'n';
  arr[2] = 'd';
  arr[3] = 'e';
  arr[4] = 'x';
  arr[5] = '.';
  arr[6] = 'h';
  arr[7] = 't';
  arr[8] = 'm';
  arr[9] = 'l';
}

int HttpRequest::getPathTokenLength(char * arr) {
  char * travel = arr;
  while(travel[0] != '/' && travel[0] != '\0') {
    travel++;
  }

  return (travel-arr);
}

void HttpRequest::setContentType(string str) {
  char * arr = (char *) str.c_str();
  arr = (arr + str.length());

  if(str.length() < 4) {
    return;
  }

  if((arr[-1] == 'g' || arr[-1] == 'G') &&
     (arr[-2] == 'p' || arr[-2] == 'P') &&
     (arr[-3] == 'j' || arr[-3] == 'J') &&
     (arr[-4] == '.')) {
    contentType = "image/jpeg";
  }

  if((arr[-1] == 'g' || arr[-1] == 'G') &&
     (arr[-2] == 'n' || arr[-2] == 'N') &&
     (arr[-3] == 'p' || arr[-3] == 'P') &&
     (arr[-4] == '.')) {
    contentType = "image/jpeg";
  }

  if(str.length() < 5) {
    return;
  }

  if((arr[-1] == 'g' || arr[-1] == 'G') &&
     (arr[-2] == 'e' || arr[-2] == 'E') &&
     (arr[-3] == 'p' || arr[-3] == 'P') &&
     (arr[-4] == 'j' || arr[-4] == 'J') &&
     (arr[-5] == '.')) {
    contentType = "image/jpeg";
  }
}
