#include "TimeMachine.h"
#include <sys/time.h>
#include <algorithm>
#include <cstddef>

unsigned long long TimeMachine::now() const
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((unsigned long long)tv.tv_sec * 1000) + tv.tv_usec / 1000;
}

std::string TimeMachine::timestamp() const
{
  time_t time = now() / 1000;
  std::string timestamp = ::ctime(&time);

  timestamp.erase(std::remove(timestamp.begin(), timestamp.end(), '\n'), timestamp.end());

  return timestamp;
}

