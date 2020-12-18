#pragma once
#include "TcpServer.h"
#include "IProxyHandler.h"
#include "ITcpServerHandler.h"
#include "Packet.h"

#include <string>
#include <map>

class ICandidateList;
class IProxy;
class IEventBus;

class Controller : public ITcpServerHandler, protected IProxyHandler
{
public:
  Controller(IProxy& proxy, IEventBus& eventBus, const ICandidateList& candidates);

  bool listen(int port);
  void onAccept(const TcpSocket& client);

  int read(TcpSocket& client);
  int ping();

protected:
  void follow(const std::string& leader);
  void propose();
  void lead();

  bool sendTo(const std::string& addr, const std::string& data);
  void packetFor(Packet::Reason reason, Packet& packet) const;

  void purge();
  void broadcast();
  void election();

  void onProxyReady();
  void onProxyNotReady();

protected:
  TcpServer _server;
  IProxy& _proxy;
  IEventBus& _eventBus;
  const ICandidateList& _candidates;
  const int _interval;

  Packet::State _state;
  unsigned long long _since;

  std::string _self;
  std::string _leader;
  std::string _missing;

  std::map<std::string,unsigned long long> _members;
  std::map<std::string,unsigned long long> _election;
  size_t _expectedCount;
};

