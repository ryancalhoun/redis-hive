#pragma once

class IEventBus
{
public:
	enum Error
	{
		None = 1,
	};

	enum Type
	{
		Stable = 0x0000, 
		Volatile = 0x0001,
	};

	class ICallback
	{
	public:
		virtual ~ICallback() {}
		virtual int operator()(int fd) = 0;
	};

	virtual ~IEventBus() {}

	virtual void add(int fd, ICallback* callback, Type type = Stable) = 0;
	virtual void removeAll(Type type) = 0;
	virtual void remove(int fd) = 0;
	virtual int next() = 0;
};

