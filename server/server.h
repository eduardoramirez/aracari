#ifndef SERVER
#define SERVER

#include "lib.h"

class Server {
  private:
    struct sockaddr_in address;

    int port;
    int sock;

    string docroot;

    bool checkCRLF(char * arr);

    void copyLast3(char * arr,char * buf1,int buf1len,char * buf2,int buf2len);
    void resetLast3(char * arr);

    int checkLast3(char * end, char * start, ssize_t bytes_read);

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
