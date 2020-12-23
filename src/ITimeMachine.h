#pragma once

#include <string>

class ITimeMachine
{
public:
  virtual ~ITimeMachine() {}

  virtual unsigned long long now() const = 0;

  virtual std::string timestamp() const = 0;
};

