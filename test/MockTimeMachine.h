#pragma once

#include "ITimeMachine.h"

class MockTimeMachine : public ITimeMachine
{
public:
  unsigned long long now() const
  {
    return _now;
  }
  std::string timestamp() const
  {
    return "";
  }

  unsigned long long _now;

};

