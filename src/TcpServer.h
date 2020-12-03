#pragma once

#include "TcpSocket.h"
class IEventBus;
class ITcpServerHandler;

class TcpServer
{
public:
  TcpServer(ITcpServerHandler& handler, IEventBus& eventBus);

  void shutdown();
  bool listen(int port);

  void accept();

protected:
  ITcpServerHandler& _handler;
  IEventBus& _eventBus;
  TcpSocket _server;
};

