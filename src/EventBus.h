#pragma once

#include "IEventBus.h"
#include <map>
#include <vector>

class EventBus : public IEventBus
{
public:
	EventBus();
	virtual ~EventBus();

	void add(int fd, ICallback* cb, Type type);
	void every(int millis, ICallback* cb);
	void removeAll(Type type);
	void remove(int fd);

	void run();

	void next();

protected:
	void scheduled();
	int timeout() const;
	unsigned long long now() const;

protected:
	int _waiter;

	struct Info
	{
		ICallback* cb;
		Type type;
	};
	std::map<int,Info> _callback;

	struct Schedule
	{
		ICallback* cb;
		int millis;
		unsigned long long last;
	};
	std::vector<Schedule> _schedule;
};

