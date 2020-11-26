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

		int operator()()
		{
			return _proxy.accept();
		}
	protected:
		Proxy& _proxy;
	};

	class Copy : public IEventBus::ICallback
	{
	public:
		Copy(Proxy& proxy, int from, int to)
			: _proxy(proxy)
			, _from(from)
			, _to(to)
		{}

		~Copy()
		{
			::close(_from);
		}

		int operator()()
		{
			return _proxy.copy(_from, _to);
		}
	protected:
		Proxy& _proxy;
		int _from;
		int _to;
	};
}

Proxy::Proxy(IEventBus& eventBus, int port)
	: _eventBus(eventBus)
	, _local(port)
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

int Proxy::getLocalPort() const
{
	return _local;
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

	int on = 1;
    ::setsockopt(_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if(::bind(_server, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		std::cout << "bind error on " << port << " (" << errno << ")" << std::endl;
		return false;
	}
	if(::listen(_server, 5) != 0) {
		return false;
	}

	struct linger l;
	l.l_onoff = 1;
	l.l_linger = 1;
	::setsockopt(_server, SOL_SOCKET, SO_LINGER, &l, sizeof(struct linger));

	_eventBus.add(_server, new Accept(*this));

	std::cout << "Proxy listening on port " << port << std::endl;

	return true;
}

int Proxy::accept()
{
	struct sockaddr_in peer;
	socklen_t len = sizeof(peer);

	int client = ::accept(_server, (struct sockaddr*)&peer, &len);
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

	_eventBus.add(client, new Copy(*this, client, sock), IEventBus::Volatile);
	_eventBus.add(sock, new Copy(*this, sock, client), IEventBus::Volatile);

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

