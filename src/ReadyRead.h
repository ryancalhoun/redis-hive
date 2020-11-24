#pragma once

#include "IReadyRead.h"
#include <map>

class ReadyRead : public IReadyRead
{
public:
	ReadyRead();
	virtual ~ReadyRead();

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

