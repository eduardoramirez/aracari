#ifndef HTTP
#define HTTP

#include "server.h"

class HttpRequest {
  private:
    string request;
    string response;

    bool beenParsed;
    bool isMalformed;
    bool accessDenied;
    bool responseGenerated;

    bool persistent;

    Server * server;


  public:
    HttpRequest(string request, Server * server);
    ~HttpRequest();

    bool parseRequest();

    string generateResponse();
};

#endif
