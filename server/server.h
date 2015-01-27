#ifndef SERVER
#define SERVER

#include <sys/types.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <stdio.h>

class Server {
  private:
    struct sockaddr_in address;

    int sock;

  public:
    Server();
    ~Server();
    
}


#endif
