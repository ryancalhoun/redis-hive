#pragma once

class IProxyHandler
{
public:
  virtual ~IProxyHandler() {}

  virtual void onProxyReady() = 0;
  virtual void onProxyNotReady() = 0;
};
