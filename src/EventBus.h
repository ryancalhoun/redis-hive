#pragma once

#include "IEventBus.h"
#include <map>

class EventBus : public IEventBus
{
public:
	EventBus();
	virtual ~EventBus();

	void add(int fd, ICallback* cb, Type type);
	void removeAll(Type type);
	void remove(int fd);

	void run();

	int next();

protected:
	int _waiter;
	struct Info
	{
		ICallback* cb;
		Type type;
	};
	std::map<int,Info> _callback;
};

