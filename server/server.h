#ifndef SERVER
#define SERVER

#include "lib.h"

class Server {
  private:
    struct sockaddr_in address;

    int port;
    int sock;

    string docroot;


    int checkLast3(char * end, char * start);

  public:
    Server(int port, string docroot);
    ~Server();

    int initialize();

    int getSock();
    int getPort();

    string getDocroot();
    
    void processRequest(int csock);

};


#endif
