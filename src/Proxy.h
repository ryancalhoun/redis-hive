#pragma once

#include "IProxy.h"

class IEventBus;
struct sockaddr_in;

class Proxy : public IProxy
{
public:
	Proxy(IEventBus& eventBus, int port);

	virtual ~Proxy();

	int getLocalPort() const;
	void address(const std::string& address, int port);
	bool listen(int port);

	void shutdown();
	void reset();

	int accept();
	int copy(int from, int to);

protected:
	IEventBus& _eventBus;
	int _server;
	int _local;
	struct sockaddr_in& _proxy;
};

