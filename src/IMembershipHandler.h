#pragma once

#include <string>

class Packet;

class IMembershipHandler
{
public:
  virtual ~IMembershipHandler() {}

  virtual bool ping(const std::string& peer, const Packet& packet) = 0;
  virtual bool ack(const Packet& packet) = 0;
};

