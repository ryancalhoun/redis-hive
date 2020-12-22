#pragma once

#include "IMembershipHandler.h"
#include "CallHistory.h"

#include "Packet.h"

class MockMembershipHandler : public IMembershipHandler
{
public:
  bool ping(const std::string& peer, const Packet& packet)
  {
    _calls.call("ping", peer, packet.serialize());
    return true;
  }
  bool ack(const Packet& packet)
  {
    _calls.call("ack", packet.serialize());
    return true;
  }

  CallHistory _calls;
};
