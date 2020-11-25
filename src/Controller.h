#pragma once

class ICandidateList;
class IProxy;
class IEventBus;

class Controller
{
public:
	Controller(IProxy& proxy, IEventBus& eventBus, const ICandidateList& candidates);

	bool listen(int port);

	int accept(int fd);
	int read(int fd);
	int ping(int fd);

protected:
	IProxy& _proxy;
	IEventBus& _eventBus;
	const ICandidateList& _candidates;

	int _server;
};

