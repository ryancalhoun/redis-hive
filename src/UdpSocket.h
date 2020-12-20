#pragma once

#include <string>

class UdpSocket
{
public:
  UdpSocket();

  bool connect(const std::string& host, int port);
  void close();

  const std::string& socketAddress() const;

protected:
  int _fd;
  std::string _socketAddress;
};

