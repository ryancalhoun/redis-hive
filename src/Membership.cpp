#include "Membership.h"
#include "ICandidateList.h"
#include "ILogger.h"
#include "IMembershipHandler.h"
#include "IProxy.h"
#include "ITimeMachine.h"

#include <algorithm>

Membership::Membership(IMembershipHandler& handler, IProxy& proxy, const ICandidateList& candidates, const ITimeMachine& timeMachine, ILogger& logger)
  : _handler(handler)
  , _proxy(proxy)
  , _candidates(candidates)
  , _timeMachine(timeMachine)
  , _logger(logger)
  , _interval(5000)
  , _state(Packet::Alone)
  , _since(_timeMachine.now())
  , _self(candidates.getSelf())
  , _expectedCount(0)
{
  _proxy.setHandler(*this);
}

namespace
{
  std::string to_s(size_t i)
  {
    char val[50] = {0};
    ::snprintf(val, sizeof(val), "%lu", i);
    return val;
  }
  std::string to_s(unsigned long long i)
  {
    char val[50] = {0};
    ::snprintf(val, sizeof(val), "%llu", i);
    return val;
  }
}

void Membership::onRead(const Packet& received)
{
  unsigned long long now = _timeMachine.now();

  if(received.self() != _self && received.reason() != Packet::Who) {
    _members[received.self()] = now;
  }

  if(received.state() == Packet::Proposing) {
    _election[received.self()] = received.since();
  } else {
    _election.erase(received.self());
  }

  if(_state == Packet::Alone || _state == Packet::Proposing) {
    if(received.following().size() == 0) {
      propose();
    } else {
      if(received.following() == _self) {
        if(_since < received.since()) {
          _since = received.since() + 1;
        }
      }
      else if(_missing.find(received.following()) == _missing.end()) {
        follow(received.following());
      }
    }

  } else if(_state == Packet::Following) {
    if(_leader == received.following()) {
      if(received.state() == Packet::Leading) {
        size_t c = _leader.find(':');
        _proxy.proxyToAddress(_leader.substr(0, c), received.proxy());

        std::vector<std::string>::const_iterator it;
        for(it = received.members().begin(); it != received.members().end(); ++it) {
          if(*it != _self) {
            _members[*it] = now;
          }
        }
      }
    } else if(_leader == received.self() && received.state() != Packet::Leading) {
      _members.erase(_leader);
      propose();
      broadcast();
    } else {
      // accept the newer
      if(_since < received.since()) {
        follow(received.following());
      }
    }

  } else if(_state == Packet::Leading) {
    if(received.state() == Packet::Leading && _self != received.self()) {
      _logger.debug("Received ping from leader " + received.self() + " (" + ::to_s(received.since()) + ") while leading (" + ::to_s(_since) + ")");
      // accept the older
      if(_since > received.since()) {
        follow(received.self());
      }
    }
  }

  if(received.reason() == Packet::Ping) {
    Packet ack;
    packetFor(Packet::Ack, ack);
    _handler.ack(ack);

  } else if(received.reason() == Packet::Ack) {
    if(_state == Packet::Proposing) {
      if(_election.size() >= _expectedCount) {
        _logger.info("Shortcut Election");
        _logger.debug("Expected election size " + ::to_s(_expectedCount) + ", actual size " + ::to_s(_election.size()));
        election();
      }
    }
  } else if(received.reason() == Packet::Who) {
    Packet who;
    packetFor(Packet::Who, who);
    std::string reply = who.serialize();
    _handler.ack(who);
  }
}

void Membership::getCandidates(std::vector<std::string>& candidates) const
{
  candidates = _candidates.getCandidates();
  std::sort(candidates.begin(), candidates.end());

  candidates.erase(std::find(candidates.begin(), candidates.end(), _self));
}

void Membership::onTimer()
{
  purge();

  Packet packet;
  packetFor(Packet::Ping, packet);

  std::vector<std::string> candidates;

  unsigned long long now = _timeMachine.now();

  if(_state == Packet::Alone || _state == Packet::Proposing) {
    getCandidates(candidates);
    std::vector<std::string>::const_iterator it;

    size_t sent = 0;
    for(it = candidates.begin(); it != candidates.end(); ++it) {
      _logger.info("Ping candidate " + *it);
      if(_handler.ping(*it, packet)) {
        ++sent;
      }
    }

    if(sent == 0) {
      if(candidates.size() == 0) {
        election();
      } else {
        propose();
      }
    }

    if(_state == Packet::Proposing) {
      if((int)(now - _since) > _interval*3) {
        election();
      }
    }

  } else if(_state == Packet::Following) {
    if(! _handler.ping(_leader, packet)) {
      if(_members.size() <= 1) {
        lead();
      } else {
        _members.erase(_leader);
        propose();
        broadcast();
      }
    }
  } else if(_state == Packet::Leading) {
    getCandidates(candidates);
    std::vector<std::string>::const_iterator it;
    for(it = candidates.begin(); it != candidates.end(); ++it) {
      if(_members.find(*it) == _members.end()) {
        _logger.info("Ping new candidate " + *it);
        _handler.ping(*it, packet);
      }
    }
  }
}

void Membership::follow(const std::string& leader)
{
  _logger.info("Follow " + leader);
  if(_leader != leader || _state != Packet::Following) {
    _since = _timeMachine.now();
  }

  _leader = leader;
  _missing.clear();
  _state = Packet::Following;
  _election.clear();
  _expectedCount = 0;
}

void Membership::propose()
{
  _logger.info("Propose");
  unsigned long long now = _timeMachine.now();

  if(_state != Packet::Proposing) {
    _since = now;
  }
  if(_leader.size() > 0) {
    _missing[_leader] = now;
    _leader.clear();
  }

  _state = Packet::Proposing;

  _election[_self] = _since;
  _expectedCount = _members.size();
  _logger.debug("Expected election size " + ::to_s(_expectedCount));
  for(std::map<std::string,unsigned long long>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
    _logger.debug("Expected member " + it->first + " (" + ::to_s(it->second) + ")");
  }
}

void Membership::lead()
{
  _logger.info("Lead");
  if(_state != Packet::Leading) {
    _since = _timeMachine.now();
    _proxy.proxyToLocal();
  }
  _state = Packet::Leading;

  _leader = _self;
  _missing.clear();
  _election.clear();
  _expectedCount = 0;
}

void Membership::packetFor(Packet::Reason reason, Packet& packet) const
{
  packet.reason(reason);
  packet.state(_state);
  packet.since(_since);
  packet.self(_self);
  packet.following(_leader);
  packet.proxy(_proxy.getLocalPort());

  if(_state == Packet::Leading) {
    std::string members;
    std::map<std::string,unsigned long long>::const_iterator it;
    for(it = _members.begin(); it != _members.end(); ++it) {
      packet.member(it->first);
    }
  }
}

namespace
{
  void purge(std::map<std::string,unsigned long long>& v, int limit, unsigned long long now)
  {
    std::map<std::string,unsigned long long>::iterator it;
    for(it = v.begin(); it != v.end();) {
      if((int)(now - it->second) > limit) {
        v.erase(it++);
      } else {
        ++it;
      }
    }
  }
}

void Membership::purge()
{
  ::purge(_members, _interval * 5, _timeMachine.now());
  ::purge(_missing, _interval * 5, _timeMachine.now());
}

void Membership::broadcast()
{
  Packet packet;
  packetFor(Packet::Ping, packet);

  std::map<std::string,unsigned long long>::const_iterator it;
  for(it = _members.begin(); it != _members.end(); ++it) {
    _handler.ping(it->first, packet);
  }
}

void Membership::election()
{
  std::map<std::string,unsigned long long>::const_iterator winner = _election.begin();
  std::map<std::string,unsigned long long>::const_iterator it;
  for(it = _election.begin(); it != _election.end(); ++it) {
    _logger.debug("Considering " + it->first + " (" + ::to_s(it->second) + ")");
    if(it->second < winner->second - 10) {
      winner = it;
    }
  }

  if(winner == _election.end() || winner->first == _self) {
    lead();
    broadcast();
  }
}

void Membership::onProxyReady()
{
  if(_state == Packet::NotReady) {
    _logger.info("Proxy ready");
    _state = Packet::Alone;
    _since = _timeMachine.now();
    broadcast();
  }
}

void Membership::onProxyNotReady()
{
  Packet::State previous = _state;

  if(_state != Packet::NotReady) {
    _logger.error("Proxy not ready");
    _state = Packet::NotReady;
    _since = _timeMachine.now();
  }

  if(previous == Packet::Leading) {
    _leader.clear();
    broadcast();
  }
}

