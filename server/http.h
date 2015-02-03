#ifndef HTTP
#define HTTP

#include "server.h"

class HttpRequest {
  private:
    string request;
    string response;
    string docroot;
    string path;
    string contentType;
    string httpType;

    bool beenParsed;
    bool isMalformed;
    bool accessDenied;
    bool notFound;
    bool responseGenerated;
    bool persistent;

    int csock;

    Server * server;

    char * skipTrim(char * arr);
    char * skipTillCRLF(char * arr);
    char * skipTillColon(char * arr);

    int getTokenLength(char * arr);

    bool parsePath(char * arr, int length);

    void request200();
    string request400();
    string request403();
    string request404();

    bool checkHttp(char * arr);
    bool checkGet(char * arr);

  public:
    HttpRequest(string request, int csock, Server * server);
    ~HttpRequest();

    void parseRequest();

    void generateResponse();

    bool isPersistent();
};

#endif
