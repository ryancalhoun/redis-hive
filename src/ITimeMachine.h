#pragma once

class ITimeMachine
{
public:
  virtual ~ITimeMachine() {}

  virtual unsigned long long now() const = 0;
};

