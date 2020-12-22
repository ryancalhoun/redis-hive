#pragma once

#include "IProxy.h"
#include "CallHistory.h"

class MockProxy : public IProxy
{
public:
  int getLocalPort() const
  {
    return 6000;
  }

  void proxyToLocal()
  {
    _calls.call("proxyToLocal");
  }

  void proxyToAddress(const std::string& address, int port)
  {
    _calls.call("proxyToAddress", address, port);
  }

  void setHandler(IProxyHandler& handler)
  {
  }

  CallHistory _calls;
};

