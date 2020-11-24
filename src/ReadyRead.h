#pragma once

#include "IReadyRead.h"
#include <map>

class ReadyRead : public IReadyRead
{
public:
	ReadyRead();
	virtual ~ReadyRead();

	void add(int fd, ICallback* cb);
	void remove(int fd);

	void run();

	int next();

protected:
	int _waiter;
	std::map<int,ICallback*> _callback;
};

