#ifndef HTACCESS
#define HTACCESS

#include "http.h"

class HTAccess {
  private:
    string directory;

    unsigned int address;

    bool allowRule(char * arr);

    char * skip(char * arr);

    int getTokenLength(char * arr);

    bool typeHostname(char * arr, int length);

    unsigned int ip_to_int(const char * ip);

  public:
    HTAccess(string directory, unsigned int address);
    ~HTAccess();

    bool allow();
};

#endif
