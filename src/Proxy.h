#pragma once

#include "IProxy.h"
#include "ITcpServerHandler.h"
#include "TcpServer.h"

#include <map>
#include <set>

class IEventBus;

class Proxy : public IProxy, public ITcpServerHandler
{
public:
  Proxy(IEventBus& eventBus, int port);

  virtual ~Proxy();

  int getLocalPort() const;
  void proxyToLocal();
  void proxyToAddress(const std::string& address, int port);
  bool listen(int port);

  void shutdown();
  void reset();

  void onAccept(const TcpSocket& client);
  void copy(TcpSocket& from, TcpSocket& to);

  std::string runCommand(const std::string& command);

protected:
  TcpServer _server;
  IEventBus& _eventBus;
  int _local;

  std::string _address;
  int _port;

  std::map<std::string, std::set<int> > _connections;
};

