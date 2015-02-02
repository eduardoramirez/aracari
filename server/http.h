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

    string request200();
    string request400();
    string request403();
    string request404();

    bool checkHttp(char * arr);
    bool checkGet(char * arr);

  public:
    HttpRequest(string request, Server * server);
    ~HttpRequest();

    void parseRequest();

    void generateResponse(int csock);

    bool isPersistent();
};

#endif
