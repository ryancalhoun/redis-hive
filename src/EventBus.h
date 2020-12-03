#pragma once

#include "IEventBus.h"
#include <map>
#include <vector>

class EventBus : public IEventBus
{
public:
  EventBus();
  virtual ~EventBus();

  void add(int fd, ICallback* cb);
  void every(int millis, ICallback* cb);
  void remove(int fd);

  void run();

  void next();

protected:
  void scheduled();
  int timeout() const;
  unsigned long long now() const;

protected:
  int _waiter;

  std::map<int,ICallback*> _callback;

  struct Schedule
  {
    ICallback* cb;
    int millis;
    unsigned long long last;
  };
  std::vector<Schedule> _schedule;
};

