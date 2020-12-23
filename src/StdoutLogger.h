#pragma once

#include "ILogger.h"

class ITimeMachine;

class StdoutLogger : public ILogger
{
public:
  StdoutLogger(const ITimeMachine& timeMachine);

  void debug(const LogMessage& message);
  void info(const LogMessage& message);
  void error(const LogMessage& message);

protected:
  const ITimeMachine& _timeMachine;
};

