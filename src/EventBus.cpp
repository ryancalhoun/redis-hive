#include "EventBus.h"
#include "Time.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <cstdlib> 
#include <errno.h>
#include <iostream>

EventBus::EventBus()
  : _waiter(::epoll_create(1))
{
}

EventBus::~EventBus()
{
  std::map<int,Info>::const_iterator it;
  for(it = _callback.begin(); it != _callback.end(); ) {
    delete it->second.cb;
    _callback.erase(it++);
  }
  ::close(_waiter);
}

void EventBus::add(int fd, ICallback* callback, Type type)
{
  struct epoll_event ev = { 0 };
  ev.data.fd = fd;
  ev.events = EPOLLIN|EPOLLPRI;

  errno = 0;
  if(::epoll_ctl(_waiter, EPOLL_CTL_ADD, fd, &ev) != 0 && errno == EEXIST) {
    ::epoll_ctl(_waiter, EPOLL_CTL_MOD, fd, &ev);
  }
  Info info = { callback, type };
  _callback[fd] = info;
}

void EventBus::every(int millis, ICallback* callback)
{
  Schedule schedule = { callback, millis, 0 };
  _schedule.push_back(schedule);
}

void EventBus::removeAll(Type type)
{
  std::map<int,Info>::const_iterator it;
  for(it = _callback.begin(); it != _callback.end(); ++it) {
    if(it->second.type == type) {
      _callback.erase(it);
    }
  }
}

void EventBus::remove(int fd)
{
  ::epoll_ctl(_waiter, EPOLL_CTL_DEL, fd, NULL);
  std::map<int,Info>::const_iterator it = _callback.find(fd);
  if(it != _callback.end()) {
    delete it->second.cb;
    _callback.erase(it);
  }
}

void EventBus::run()
{
  for(;;) {
    next();
  }
}

void EventBus::scheduled()
{
  unsigned long long now = Time().now();
  std::vector<Schedule>::iterator it;
  for(it = _schedule.begin(); it != _schedule.end(); ++it) {
    if(it->last == 0 || (int)(now - it->last) >= it->millis) {
      (*it->cb)();
      it->last = now;
    }
  }
}

void EventBus::next()
{
  struct epoll_event ev = { 0 };

  if(::epoll_wait(_waiter, &ev, 1, timeout()) != -1) {
    std::map<int,Info>::const_iterator it = _callback.find(ev.data.fd);
    if(it != _callback.end()) {
      (*it->second.cb)();
    }
  }

  scheduled();
}

int EventBus::timeout() const
{
  int min = -1;
  unsigned long long now = Time().now();
  std::vector<Schedule>::const_iterator it;
  for(it = _schedule.begin(); it != _schedule.end(); ++it) {
    int next = it->millis;
    if(it->last != 0) {
      int elapsed = (int)(now - it->last);
      if(elapsed < next) {
        next -= elapsed;
      }
    }
    if(min == -1 || next < min) {
      min = next;
    }
  }
  return min;
}

