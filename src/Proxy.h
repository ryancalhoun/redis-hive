#pragma once

#include "IProxy.h"
#include "ITcpServerHandler.h"
#include "TcpServer.h"

#include <map>
#include <set>
#include <deque>

class IEventBus;
class ILogger;

class Proxy : public IProxy, public ITcpServerHandler
{
public:
  Proxy(IEventBus& eventBus, int port, ILogger& logger);

  virtual ~Proxy();

  int getLocalPort() const;
  void proxyToLocal();
  void proxyToAddress(const std::string& address, int port);
  bool listen(int port);

  void setHandler(IProxyHandler& handler);

  void shutdown();
  void reset();
  void onAccept(const TcpSocket& client);

  void copy(TcpSocket& from, TcpSocket& to);

protected:
  void ping();
  void pong();
  void runCommand(const std::string& command);
  void runNextCommand();
  void ready();
  void notReady();

protected:
  TcpServer _server;
  TcpSocket _redis;
  IEventBus& _eventBus;
  ILogger& _logger;
  IProxyHandler* _handler;
  const int _local;
  const int _interval;

  std::deque<std::string> _commands;
  bool _commandInFlight;

  std::string _address;
  int _port;
  bool _alone;

  std::map<std::string, std::set<int> > _connections;
};

