#pragma once

#include "LogMessage.h"

class ILogger
{
public:
  virtual ~ILogger() {}

  virtual void debug(const LogMessage& msg) = 0;
  virtual void info(const LogMessage& msg) = 0;
  virtual void error(const LogMessage& msg) = 0;
};

