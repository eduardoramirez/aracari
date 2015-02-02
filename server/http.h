#ifndef HTTP
#define HTTP

#include "server.h"

class HttpRequest {
  private:
    string request;
    string response;

    string docroot;

    string path;

    bool beenParsed;
    bool isMalformed;
    bool accessDenied;
    bool notFound;
    bool responseGenerated;

    bool persistent;

    Server * server;

    char * skipTrim(char * arr);

    int getTokenLength(char * arr);

    bool parsePath(char * arr, int length);

    string badRequest();
    string request404();
    string goodRequest();

  public:
    HttpRequest(string request, Server * server);
    ~HttpRequest();

    void parseRequest();

    void generateResponse(int csock);
};

#endif
