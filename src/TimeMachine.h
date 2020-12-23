#pragma once

#include "ITimeMachine.h"

class TimeMachine : public ITimeMachine
{
public:
  unsigned long long now() const;
  std::string timestamp() const;
};
