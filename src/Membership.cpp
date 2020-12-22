#include "Membership.h"
#include "ICandidateList.h"
#include "IMembershipHandler.h"
#include "IProxy.h"
#include "ITimeMachine.h"

#include <cstdio> 
#include <cstdlib> 
#include <cstring> 

#include <algorithm>
#include <iostream>

Membership::Membership(IMembershipHandler& handler, IProxy& proxy, const ICandidateList& candidates, const ITimeMachine& timeMachine)
  : _handler(handler)
  , _proxy(proxy)
  , _candidates(candidates)
  , _timeMachine(timeMachine)
  , _interval(5000)
  , _state(Packet::Alone)
  , _since(_timeMachine.now())
  , _self(candidates.getSelf())
  , _expectedCount(0)
{
  _proxy.setHandler(*this);
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
        std::cout << "Shortcut election" << std::endl;
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

void Membership::onTimer()
{
  purge();

  Packet packet;
  packetFor(Packet::Ping, packet);

  unsigned long long now = _timeMachine.now();

  if(_state == Packet::Alone || _state == Packet::Proposing) {
    std::vector<std::string> candidates = _candidates.getCandidates();

    std::sort(candidates.begin(), candidates.end());

    size_t sent = 0;
    std::vector<std::string>::const_iterator it;
    for(it = candidates.begin(); it != candidates.end(); ++it) {
      if(*it != _self) {
        if(_handler.ping(*it, packet)) {
          ++sent;
        }
      }
    }

    if(sent == 0) {
      if(candidates.size() == 1 && candidates[0] == _self) {
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
  }
}

void Membership::follow(const std::string& leader)
{
  std::cout << "Follow " << leader << std::endl;
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
  std::cout << "Propose" << std::endl;
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
}

void Membership::lead()
{
  std::cout << "Lead" << std::endl;
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
    std::cout << "Proxy Ready" << std::endl;
    _state = Packet::Alone;
    _since = _timeMachine.now();
    broadcast();
  }
}

void Membership::onProxyNotReady()
{
  Packet::State previous = _state;

  if(_state != Packet::NotReady) {
    std::cout << "Proxy Not Ready" << std::endl;
    _state = Packet::NotReady;
    _since = _timeMachine.now();
  }

  if(previous == Packet::Leading) {
    _leader.clear();
    broadcast();
  }
}

