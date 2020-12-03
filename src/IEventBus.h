#pragma once

class IEventBus
{
public:
  enum Type
  {
    Stable = 0x0000, 
    Volatile = 0x0001,
  };

  class ICallback
  {
  public:
    virtual ~ICallback() {}
    virtual int operator()() = 0;
  };

  virtual ~IEventBus() {}

  virtual void add(int fd, ICallback* callback, Type type = Stable) = 0;
  virtual void every(int millis, ICallback* callback) = 0;
  virtual void removeAll(Type type) = 0;
  virtual void remove(int fd) = 0;
};

