#include "htaccess.h"

HTAccess::HTAccess(string directory, unsigned int address) {
  this->directory = "";
  this->directory = this->directory + directory;

  this->address = address;
}

HTAccess::~HTAccess() {
}

bool HTAccess::allow() {
  string fileName = directory + "/.htaccess";

  FILE * file;

  if((file = fopen(fileName.c_str(), "rb")) == NULL) {
    fprintf(stderr, "Couldn't open .htaccess file\n");
    return true; // Allow connection
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  fseek(file, 0L, SEEK_SET);

  char buf[fileSize+1];

  for(unsigned int i = 0; i < fileSize+1; i++) {
    buf[i] = 0;
  }

  while(!feof(file)) {
    fread(buf, 1, fileSize, file);
  }

  fclose(file);

  char * travel = buf;

  while(travel[0] != '\0') {
    bool allowed;

    if(allowRule(travel)) {
      travel = travel + 11;
      allowed = true;
    }
    else {
      travel = travel + 10;
      allowed = false;
    }

    int tokenLength = getTokenLength(travel);

    if(typeHostname(travel, tokenLength)) {
      string token(travel, tokenLength);

      travel = travel + tokenLength;

      int status;
      struct addrinfo hints;
      struct addrinfo *servinfo;

      memset(&hints, 0, sizeof hints);

      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;

      if ((status = getaddrinfo(token.c_str(), NULL, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        travel = skip(travel);
        continue;
      }
  
      struct addrinfo * p;

      for(p = servinfo; p != NULL; p = p->ai_next) {
        struct sockaddr_in * ipv4 = (struct sockaddr_in *)p->ai_addr;

        unsigned int checker = ipv4->sin_addr.s_addr;

        if(checker == address) {
          freeaddrinfo(servinfo);
          return allowed;
        }
      }

      freeaddrinfo(servinfo);
    }
    else {
      int ipLength = tokenLength;

      while(travel[ipLength] != '/') {
        ipLength--;
      }

      string token1(travel, ipLength);
      unsigned int checkAddress = ip_to_int(token1.c_str());

      travel = travel + ipLength + 1;

      string token2(travel, tokenLength-(ipLength+1));
      int maskCount = stoi(token2, NULL, 10);

      travel = travel + (tokenLength - (ipLength+1));

      unsigned int value = 2147483648;
      unsigned int mask = 0;

      while(maskCount > 0) {
        mask += value;
        value /= 2;
      }

      if((mask & address) == checkAddress) {
        return allowed;
      }
    }
    travel = skip(travel);
  }

  return true;
}

bool HTAccess::allowRule(char * arr) {
  if(arr[0] == 'a' || arr[0] == 'A') {
    return true;
  }
  return false;
}

char * HTAccess::skip(char * arr) {
  while(arr[0] == '\t'  ||
      arr[0] == '\r' ||
      arr[0] == '\n' ||
      arr[0] == ' ') {
    arr++;
  }

  return arr;
}

int HTAccess::getTokenLength(char * arr) {
  char * travel = arr;

  while(travel[0] != '\r' &&
      travel[0] != '\n' &&
      travel[0] != '\t'  &&
      travel[0] != ' ') {
    travel++;
  }

  return (travel - arr);
}

bool HTAccess::typeHostname(char * arr, int length) {
  int dotCount = 0;
  int slashCount = 0;

  for(int i = 0; i < length; i++) {
    if(arr[i] == '.') {
      dotCount++;
    }
    else if(arr[i] == '/') {
      slashCount++;
    }
    else if(arr[i] < '0' || arr[i] > '9') {
      return true;
    }
  }

  if(dotCount == 3 && slashCount == 1) {
    return false;
  }

  return true;
}


/* 
  Please Note: This is one function I found online for quick conversion
               of ip to an unsigned int. I hope I'll be allowed to use this
 */
unsigned int HTAccess::ip_to_int(const char * ip)
{
  unsigned v = 0;
  int i;
  const char * start;

  start = ip;
  for (i = 0; i < 4; i++) {
    char c;
    int n = 0;
    while (1) {
      c = * start;
      start++;
      if (c >= '0' && c <= '9') {
        n *= 10;
        n += c - '0';
      }
      else if ((i < 3 && c == '.') || i == 3) {
        break;
      }
      else {
        return INVALID;
      }
    }
    if (n >= 256) {
      return INVALID;
    }
    v *= 256;
    v += n;
  }
  return v;
}

