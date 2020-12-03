#include "Packet.h"

Packet::Packet()
{
  clear();
}

Packet::Packet(const std::string& data)
{
  parse(data);
}

void Packet::clear()
{
  _state = Alone;
  _reason = None;
  _since = 0;
  _self.clear();
  _following.clear();
  _proxy = 0;
  _members.clear();
}

namespace
{
  std::string to_s(unsigned long long i)
  {
    char val[50] = {0};
    ::snprintf(val, sizeof(val), "%llu", i);
    return val;
  }
}

std::string Packet::serialize() const
{
  const char* states = "APLF";
  std::string data;
  data += "a=" + _self;
  if(_reason == Ping) {
    data += "|e=ping";
  } else if(_reason == Ack) {
    data += "|e=ack";
  }
  data += "|s=" + std::string(1, states[_state]);
  data += "|f=" + _following;
  data += "|r=" + ::to_s(_proxy);
  data += "|t=" + ::to_s(_since);

  std::vector<std::string>::const_iterator it;
  for(it = _members.begin(); it != _members.end(); ++it) {
    if(it == _members.begin()) {
      data += "|m=";
    } else {
      data += ",";
    }
    data += *it;
  }

  return data;
}

Packet::State Packet::state() const
{
  return _state;
}

Packet::Reason Packet::reason() const
{
  return _reason;
}

unsigned long long Packet::since() const
{
  return _since;
}

const std::string& Packet::self() const
{
  return _self;
}

const std::string& Packet::following() const
{
  return _following;
}

int Packet::proxy() const
{
  return _proxy;
}

const std::vector<std::string>& Packet::members() const
{
  return _members;
}

bool Packet::parse(const std::string& data)
{
  clear();
  size_t begin = 0, end = 0;
  for(; end != std::string::npos; begin = end + 1) {
    end = data.find('|', begin);
    std::string part = data.substr(begin, end-begin);
    if(part.size() > 2) {
      std::string val = part.substr(2);
      switch(part[0]) {
        case 'e': 
          if(val == "ping") {
            _reason = Ping;
          } else if(val == "ack") {
            _reason = Ack;
          }
        break;
        case 's':
          if(val == "P") {
            _state = Proposing;
          } else if(val == "L") {
            _state = Leading;
          } else if(val == "F") {
            _state = Following;
          }
        break;
        case 't':
          _since = ::strtoull(val.c_str(), NULL, 10);
        break;
        case 'a':
          _self = val;
        break;
        case 'f':
          _following = val;
        break;
        case 'r':
          _proxy = ::strtol(val.c_str(), NULL, 10);
        break;
        case 'm':
          size_t begin = 0, end = 0;
          for(; end != std::string::npos; begin = end + 1) {
            end = val.find(',', begin);
            std::string member = val.substr(begin, end - begin);
            if(member.size() > 0) {
              _members.push_back(member);
            }
          }
        break;
      }
    } else {
      return false;
    }
  }
  return true;
}

void Packet::state(State state)
{
  _state = state;
}

void Packet::reason(Reason reason)
{
  _reason = reason;
}

void Packet::since(unsigned long long since)
{
  _since = since;
}

void Packet::self(const std::string& self)
{
  _self = self;
}

void Packet::following(const std::string& leader)
{
  _following = leader;
}

void Packet::proxy(int port)
{
  _proxy = port;
}

void Packet::members(const std::vector<std::string>& members)
{
  _members = members;
}

void Packet::member(const std::string& member)
{
  _members.push_back(member);
}

