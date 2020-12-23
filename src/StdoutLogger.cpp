#include "StdoutLogger.h"
#include "LogMessage.h"
#include "ITimeMachine.h"

#include <iostream>

StdoutLogger::StdoutLogger(const ITimeMachine& timeMachine)
  : _timeMachine(timeMachine)
{}

void StdoutLogger::debug(const LogMessage& msg)
{
  std::cout << _timeMachine.timestamp() << " [D] " << msg << std::endl;
}

void StdoutLogger::info(const LogMessage& msg)
{
  std::cout << _timeMachine.timestamp() << " [I] " << msg << std::endl;
}

void StdoutLogger::error(const LogMessage& msg)
{
  std::cerr << _timeMachine.timestamp() << " [E] " << msg.str() << std::endl;
}

