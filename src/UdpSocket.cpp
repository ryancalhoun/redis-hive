#include "UdpSocket.h"
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>

UdpSocket::UdpSocket()
  : _fd(-1)
{}

bool UdpSocket::connect(const std::string& host, int port)
{
  _fd = ::socket(PF_INET, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, IPPROTO_UDP);
  if(_fd == -1) {
    return false;
  }

  struct sockaddr_in peer;
  peer.sin_family = AF_INET; 
  ::inet_aton(host.c_str(), &peer.sin_addr);
  peer.sin_port = htons(port); 

  if(::connect(_fd, (const struct sockaddr*)&peer, sizeof(peer)) != 0) {
    close();
    return false;
  }

  struct sockaddr_in self;
  socklen_t addrlen = sizeof(self);

  if(::getsockname(_fd, (struct sockaddr*)&self, &addrlen) != 0) {
    close();
    return false;
  }

  _socketAddress = ::inet_ntoa(self.sin_addr);

  return true;
}

void UdpSocket::close()
{
  ::close(_fd);
  _fd = -1;
}

const std::string& UdpSocket::socketAddress() const
{
  return _socketAddress;
}

