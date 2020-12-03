#pragma once

class TcpSocket;

class ITcpServerHandler
{
public:
  virtual ~ITcpServerHandler() {}

  virtual void onAccept(const TcpSocket& client) = 0;
};

