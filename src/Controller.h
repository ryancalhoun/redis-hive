#pragma once
#include "Membership.h"
#include "TcpServer.h"
#include "IMembershipHandler.h"
#include "ITcpServerHandler.h"
#include "Packet.h"

#include <string>

class ICandidateList;
class IEventBus;
class IProxy;
class ITimeMachine;

class Controller : public ITcpServerHandler, protected IMembershipHandler
{
public:
  Controller(IProxy& proxy, IEventBus& eventBus, const ICandidateList& candidates, const ITimeMachine& timeMachine);

  bool listen(int port);
  void onAccept(const TcpSocket& client);

  int read(TcpSocket& client);
  int ping();

protected:
  bool ping(const std::string& addr, const Packet& packet);
  bool ack(const Packet& packet);

protected:
  TcpServer _server;
  Membership _membership;
  IEventBus& _eventBus;
  const int _interval;

  TcpSocket* _currentClient;
};

