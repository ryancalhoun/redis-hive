#pragma once

#include "IProxy.h"

class MockProxy : public IProxy
{
public:
  int getLocalPort() const
  {
    return 6000;
  }

  void proxyToLocal()
  {
  }

  void proxyToAddress(const std::string& address, int port)
  {
  }

  void setHandler(IProxyHandler& handler)
  {
  }
};

