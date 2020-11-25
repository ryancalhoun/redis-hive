#include "Controller.h"
#include "ICandidateList.h"
#include "IProxy.h"
#include "IEventBus.h"

#include <cstdio> 
#include <cstdlib> 
#include <cstring> 

#include <iostream>

#include <netinet/in.h> 
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <arpa/inet.h>

namespace
{
	class CB : public IEventBus::ICallback
	{
	public:
		CB(Controller& controller, int (Controller::*f)(int), int fd)
			: _controller(controller)
			, _f(f)
			, _fd(fd)
		{}

		~CB()
		{
			::close(_fd);
		}

		int operator()()
		{
			return (_controller.*_f)(_fd);
		}
	protected:
		Controller& _controller;
		int (Controller::*_f)(int);
		int _fd;
	};
}

Controller::Controller(IProxy& proxy, IEventBus& eventBus, const ICandidateList& candidates)
	: _proxy(proxy)
	, _eventBus(eventBus)
	, _candidates(candidates)
{}

bool Controller::listen(int port)
{
	struct sockaddr_in addr = { 0 };
	_server = ::socket(AF_INET, SOCK_STREAM, 0);

	if(_server == -1) {
		return false;
	}

	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = ::htonl(INADDR_ANY); 
	addr.sin_port = htons(port); 

	if(::bind(_server, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		return false;
	}
	if(::listen(_server, 5) != 0) {
		return false;
	}

	_eventBus.add(_server, new CB(*this, &Controller::accept, _server));
	_eventBus.every(5000, new CB(*this, &Controller::ping, 0));

	std::cout << "Controller listending on port " << port << std::endl;

	return true;
}

int Controller::accept(int fd)
{
	struct sockaddr_in peer;
	socklen_t len = sizeof(peer);

	int client = ::accept(fd, (struct sockaddr*)&peer, &len);
	if(client == -1) {
		return -1;
	}

	_eventBus.add(client, new CB(*this, &Controller::read, client));
	return 0;
}

int Controller::read(int fd)
{
	return 0;
}

int Controller::ping(int fd)
{
	std::cout << "Ping " << fd << std::endl;
	return 0;
}

