#pragma once

#include "IMembershipHandler.h"

class MockMembershipHandler : public IMembershipHandler
{
public:
  bool ping(const std::string& peer, const Packet& packet)
  {
    return true;
  }
  bool ack(const Packet& packet)
  {
    return true;
  }
};
