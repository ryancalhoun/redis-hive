#include "Controller.h"
#include "ICandidateList.h"
#include "IProxy.h"
#include "IEventBus.h"
#include "Time.h"
#include "TcpSocket.h"

#include <cstdio> 
#include <cstdlib> 
#include <cstring> 

#include <algorithm>
#include <iostream>

#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <arpa/inet.h>

namespace
{
  class Ping : public IEventBus::ICallback
  {
  public:
    Ping(Controller& controller) : _controller(controller) {}
    void operator()() { _controller.ping(); }

  protected:
    Controller& _controller;
  };

  class Read : public IEventBus::ICallback
  {
  public:
    Read(Controller& controller, const TcpSocket& client)
      : _controller(controller)
      , _client(client)
    {}

    ~Read() { _client.close(); }

    void operator()() { _controller.read(_client); }

  protected:
    Controller& _controller;
    TcpSocket _client;
  };
}

Controller::Controller(IProxy& proxy, IEventBus& eventBus, const ICandidateList& candidates)
  : _server(*this, eventBus)
  , _proxy(proxy)
  , _eventBus(eventBus)
  , _candidates(candidates)
  , _interval(5000)
  , _state(Packet::Alone)
  , _since(Time().now())
  , _self(candidates.getSelf())
  , _expectedCount(0)
{
  _proxy.setHandler(*this);
}

void Controller::onAccept(const TcpSocket& client)
{
  _eventBus.add(client, new Read(*this, client));
}

bool Controller::listen(int port)
{
  if(! _server.listen(port)) {
    return false;
  }

  _eventBus.every(_interval, new Ping(*this));

  std::cout << "Controller listening on port " << port << std::endl;

  return true;
}

int Controller::read(TcpSocket& client)
{
  char buf[1024] = {0};
  if(! client.read(buf, sizeof(buf))) {
    return -1;
  }

  if(client.bytes() == 0) {
    _eventBus.remove(client);
  } else {
    Packet received(buf);
    unsigned long long now = Time().now();

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
        if(received.following() != _missing) {
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
      } else if(_leader == received.self() && received.state() == Packet::NotReady) {
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
      std::string reply = ack.serialize();
      client.write(reply.c_str(), reply.size());
      _eventBus.remove(client);
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
      client.write(reply.c_str(), reply.size());
      _eventBus.remove(client);
    }
  }

  return 0;
}

int Controller::ping()
{
  purge();

  Packet packet;
  packetFor(Packet::Ping, packet);

  unsigned long long now = Time().now();

  if(_state == Packet::Alone || _state == Packet::Proposing) {
    std::vector<std::string> candidates = _candidates.getCandidates();

    std::sort(candidates.begin(), candidates.end());

    size_t sent = 0;
    std::vector<std::string>::const_iterator it;
    for(it = candidates.begin(); it != candidates.end(); ++it) {
      if(*it != _self) {
        if(sendTo(*it, packet.serialize())) {
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
    if(! sendTo(_leader, packet.serialize())) {
      if(_members.size() <= 1) {
        lead();
      } else {
        _members.erase(_leader);
        propose();
        broadcast();
      }
    }
  }

  return 0;
}

void Controller::follow(const std::string& leader)
{
  std::cout << "Follow " << leader << std::endl;
  if(_leader != leader || _state != Packet::Following) {
    _since = Time().now();
  }

  _leader = leader;
  _missing.clear();
  _state = Packet::Following;
  _election.clear();
  _expectedCount = 0;
}

void Controller::propose()
{
  std::cout << "Propose" << std::endl;
  if(_state != Packet::Proposing) {
    _since = Time().now();
  }
  if(_leader.size() > 0) {
      _missing = _leader;
  }
  _leader = "";
  _state = Packet::Proposing;
  _election[_self] = _since;
  _expectedCount = _members.size();
}

void Controller::lead()
{
  std::cout << "Lead" << std::endl;
  if(_state != Packet::Leading) {
    _since = Time().now();
    _proxy.proxyToLocal();
  }
  _leader = _self;
  _missing.clear();
  _state = Packet::Leading;
  _election.clear();
  _expectedCount = 0;
}

bool Controller::sendTo(const std::string& addr, const std::string& data)
{
  TcpSocket sock;
  if(! sock.connect(addr) || ! sock.write(data.c_str(), data.size())) {
    sock.close();
    return false;
  }

  _eventBus.add(sock, new Read(*this, sock));
  return true;
}

void Controller::packetFor(Packet::Reason reason, Packet& packet) const
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

void Controller::purge()
{
  unsigned long long now = Time().now();

  std::map<std::string,unsigned long long>::iterator it;
  for(it = _members.begin(); it != _members.end();) {
    if((int)(now - it->second) > _interval * 5) {
      _members.erase(it++);
    } else {
      ++it;
    }
  }
}

void Controller::broadcast()
{
  Packet packet;
  packetFor(Packet::Ping, packet);

  std::map<std::string,unsigned long long>::const_iterator it;
  for(it = _members.begin(); it != _members.end(); ++it) {
    sendTo(it->first, packet.serialize());
  }
}

void Controller::election()
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

void Controller::onProxyReady()
{
  if(_state == Packet::NotReady) {
    std::cout << "Proxy Ready" << std::endl;
    _state = Packet::Alone;
    _since = Time().now();
    broadcast();
  }
}

void Controller::onProxyNotReady()
{
  Packet::State previous = _state;

  if(_state != Packet::NotReady) {
    std::cout << "Proxy Not Ready" << std::endl;
    _state = Packet::NotReady;
    _since = Time().now();
  }

  if(previous == Packet::Leading) {
    _leader.clear();
    broadcast();
  }
}

