#pragma once

#include <vector>
#include <string>

class Packet
{
public:
  enum State
  {
    Alone,
    Proposing,
    Leading,
    Following,
    NotReady,
  };
  enum Reason
  {
    None,
    Ping,
    Ack,
    Who,
  };

  Packet();
  Packet(const std::string& data);

  void clear();

  std::string serialize() const;
  State state() const;
  Reason reason() const;
  unsigned long long since() const;
  const std::string& self() const;
  const std::string& following() const;
  int proxy() const;
  const std::vector<std::string>& members() const;

  bool parse(const std::string& data);
  void state(State state);
  void reason(Reason reason);
  void since(unsigned long long since);
  void self(const std::string& self);
  void following(const std::string& leader);
  void proxy(int port);
  void members(const std::vector<std::string>& members);
  void member(const std::string& member);

protected:
  State _state;
  Reason _reason;
  unsigned long long _since;
  std::string _self;
  std::string _following;
  int _proxy;
  std::vector<std::string> _members;
};

