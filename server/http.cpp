#include "http.h"

HttpRequest::HttpRequest(string request, Server * server) {
  fprintf(stderr, "Request received. Printing it out.\n");
  fprintf(stderr, "%s\n", request.c_str());

  this->request = request;
  this->server = server;
  server->getDocroot();

  beenParsed = false;
  isMalformed = false;
  accessDenied = false;
  responseGenerated = false;

  persistent = false;
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::parseRequest() {
  return false;
}

string HttpRequest::generateResponse() {
  return NULL;
}
