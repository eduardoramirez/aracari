#include "http.h"

HttpRequest::HttpRequest(string request, Server * server) {
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
