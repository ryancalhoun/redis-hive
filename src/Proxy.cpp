#include "Proxy.h"
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
	class Accept : public IEventBus::ICallback
	{
	public:
		Accept(Proxy& proxy)
			: _proxy(proxy)
		{}

		int operator()(int fd)
		{
			return _proxy.accept(fd);
		}
	protected:
		Proxy& _proxy;
	};

	class Copy : public IEventBus::ICallback
	{
	public:
		Copy(Proxy& proxy, int to)
			: _proxy(proxy)
			, _to(to)
		{}

		~Copy()
		{
			::close(_to);
		}

		int operator()(int fd)
		{
			return _proxy.copy(fd, _to);
		}
	protected:
		Proxy& _proxy;
		int _to;
	};
}

Proxy::Proxy(IEventBus& eventBus)
	: _eventBus(eventBus)
	, _proxy(*new struct sockaddr_in)
{
}

Proxy::~Proxy()
{
	shutdown();
	delete &_proxy;
}

void Proxy::shutdown()
{
	reset();

	_eventBus.remove(_server);
	::close(_server);
}

void Proxy::address(const std::string& address, int port)
{
	::inet_aton(address.c_str(), &_proxy.sin_addr);
	_proxy.sin_family = AF_INET; 
	_proxy.sin_port = htons(port); 
}

void Proxy::reset()
{
	_eventBus.removeAll(IEventBus::Volatile);
}

bool Proxy::listen(int port)
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

	_eventBus.add(_server, new Accept(*this));

	std::cout << "Proxy listending on port " << port << std::endl;

	return true;
}

int Proxy::accept(int fd)
{
	struct sockaddr_in peer;
	socklen_t len = sizeof(peer);

	int client = ::accept(fd, (struct sockaddr*)&peer, &len);
	if(client == -1) {
		return -1;
	}

	int sock = ::socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		::close(client);
		return -1;
	}

	if(::connect(sock, (struct sockaddr*)&_proxy, sizeof(_proxy)) != 0) {
		std::cout << "connect error " << errno << std::endl;
		::close(client);
		::close(sock);
		return -1;
	}

	_eventBus.add(client, new Copy(*this, sock), IEventBus::Volatile);
	_eventBus.add(sock, new Copy(*this, client), IEventBus::Volatile);

	return 0;
}

int Proxy::copy(int from, int to)
{
	char buf[1024];
	ssize_t bytes = ::recv(from, buf, sizeof(buf), MSG_DONTWAIT);

	if(bytes == 0) {
		_eventBus.remove(from);
		_eventBus.remove(to);
	} else if(bytes > 0) {
		ssize_t sent = ::send(to, buf, bytes, 0);
		if(sent != bytes) {
			return -1;
		}
	}

	return 0;
}

