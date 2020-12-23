#pragma once

#include "ILogger.h"

class MockLogger : public ILogger
{
public:
  void debug(const LogMessage& msg)
  {
  }

  void info(const LogMessage& msg)
  {
  }

  void error(const LogMessage& msg)
  {
  }
};
