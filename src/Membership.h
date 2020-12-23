#pragma once
#include "IProxyHandler.h"
#include "Packet.h"

#include <string>
#include <map>

class ICandidateList;
class ILogger;
class IMembershipHandler;
class IProxy;
class ITimeMachine;

class Membership : protected IProxyHandler
{
public:
  Membership(IMembershipHandler& handler, IProxy& proxy, const ICandidateList& candidates, const ITimeMachine& timeMachine, ILogger& logger);

  void onRead(const Packet& received);
  void onTimer();

protected:
  void follow(const std::string& leader);
  void propose();
  void lead();

  void packetFor(Packet::Reason reason, Packet& packet) const;

  void purge();
  void broadcast();
  void election();

  void onProxyReady();
  void onProxyNotReady();

protected:
  IMembershipHandler& _handler;
  IProxy& _proxy;
  const ICandidateList& _candidates;
  const ITimeMachine& _timeMachine;
  ILogger& _logger;
  const int _interval;

  Packet::State _state;
  unsigned long long _since;

  std::string _self;
  std::string _leader;

  std::map<std::string,unsigned long long> _members;
  std::map<std::string,unsigned long long> _missing;
  std::map<std::string,unsigned long long> _election;
  size_t _expectedCount;
};

