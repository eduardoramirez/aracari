#ifndef SERVER
#define SERVER

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <string>

#include <sys/stat.h>

using namespace std;

class Server {
  private:
    struct sockaddr_in address;

    int port;
    int sock;

    string docroot;

  public:
    Server(int port, string docroot);
    ~Server();

    int initialize();

    int getSock();
    int getPort();

    string getDocroot();
    
    int processRequest(int csock);

};


#endif
