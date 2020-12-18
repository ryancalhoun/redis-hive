#include "TcpSocket.h"

#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>

TcpSocket::TcpSocket()
  : _fd(-1)
  , _ec(0)
{}

TcpSocket::TcpSocket(int fd)
  : _fd(fd)
  , _ec(0)
{}

TcpSocket::operator int() const
{
  return _fd;
}

bool TcpSocket::listen(int port)
{
  struct sockaddr_in addr = { 0 };
  addr.sin_family = AF_INET; 
  addr.sin_addr.s_addr = ::htonl(INADDR_ANY); 
  addr.sin_port = htons(port); 

  _fd = ::socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, 0);
  if(_fd == -1) {
    _ec = errno;
    return false;
  }

  int on = 1;
    ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  if(::bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    std::cout << "bind error on " << port << " (" << errno << ")" << std::endl;
    _ec = errno;
    return false;
  }
  if(::listen(_fd, 5) != 0) {
    return false;
  }
  struct linger l;
  l.l_onoff = 1;
  l.l_linger = 1;
  ::setsockopt(_fd, SOL_SOCKET, SO_LINGER, &l, sizeof(struct linger));
  return true;
}

int TcpSocket::accept()
{
  struct sockaddr_in peer;
  socklen_t len = sizeof(peer);

  int client = ::accept4(_fd, (struct sockaddr*)&peer, &len, SOCK_CLOEXEC);
  if(client == -1) {
    _ec = errno;
    return -1;
  }
  return client;
}

bool TcpSocket::connect(const std::string& addr)
{
  size_t c = addr.find(':');
  int port = atoi(addr.substr(c+1).c_str());
  return connect(addr.substr(0, c), port);
}

bool TcpSocket::connect(const std::string& host, int port)
{
  struct sockaddr_in addr;
  addr.sin_family = AF_INET; 

  ::inet_aton(host.c_str(), &addr.sin_addr);
  addr.sin_port = htons(port); 

  _fd = ::socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, 0);
  if(::connect(_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    _ec = errno;
    close();
    return false;
  }

  return true;
}

bool TcpSocket::read(void* buff, size_t length)
{
  ssize_t bytes = ::recv(_fd, buff, length, MSG_DONTWAIT);
  if(bytes < 0) {
    _ec = errno;
    return false;
  }
  _bytes = bytes;
  return true;
}

bool TcpSocket::write(const void* buff, size_t length)
{
  ssize_t bytes = ::send(_fd, buff, length, 0);
  if((size_t)bytes != length) {
    _ec = errno;
    return false;
  }
  return true;
}

size_t TcpSocket::bytes() const
{
  return _bytes;
}

void TcpSocket::close()
{
  if(_fd != -1) {
    ::close(_fd);
    _fd = -1;
  }
}


