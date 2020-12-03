#include "Time.h"
#include <sys/time.h>
#include <cstddef>

unsigned long long Time::now() const
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((unsigned long long)tv.tv_sec * 1000) + tv.tv_usec / 1000;
}

