#include "DnsLookup.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <netinet/in.h>
#include <arpa/inet.h>

bool DnsLookup::lookup(const std::string& name, std::vector<std::string>& ips) const
{
  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo* r = NULL;

  if(::getaddrinfo(name.c_str(), NULL, &hints, &r) != 0) {
    return false;
  }

  for(; r; r = r->ai_next) {
    ips.push_back(::inet_ntoa(((struct sockaddr_in*)r->ai_addr)->sin_addr));
  }

  ::freeaddrinfo(r);

  return true;
}

