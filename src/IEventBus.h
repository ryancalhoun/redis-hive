#pragma once

class IEventBus
{
public:
  class ICallback
  {
  public:
    virtual ~ICallback() {}
    virtual int operator()() = 0;
  };

  virtual ~IEventBus() {}

  virtual void add(int fd, ICallback* callback) = 0;
  virtual void every(int millis, ICallback* callback) = 0;
  virtual void remove(int fd) = 0;
};

