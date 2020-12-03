#pragma once

#include <string>

class IProxy
{
public:
  virtual ~IProxy() {}

  virtual int getLocalPort() const = 0;
  virtual void proxyToLocal() = 0;
  virtual void proxyToAddress(const std::string& address, int port) = 0;
  virtual void reset() = 0;
};

