#include "lib.h"


char CR = '\r';
char LF = '\n';

bool is_file(char * path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISREG(buf.st_mode);
}

bool is_dir(char * path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

void shut(int csock, int ssock) {
  close(csock);
  close(ssock);
  exit(0);
}

void shut(int sock) {
  close(sock);
  exit(0);
}
