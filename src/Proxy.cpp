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
	class Copy : public IEventBus::ICallback
	{
	public:
		Copy(Proxy& proxy, const TcpSocket& from, const TcpSocket& to)
			: _proxy(proxy)
			, _from(from)
			, _to(to)
		{}

		~Copy()
		{
			_from.close();
		}

		int operator()()
		{
			_proxy.copy(_from, _to);
			return 0;
		}
	protected:
		Proxy& _proxy;
		TcpSocket _from;
		TcpSocket _to;
	};
}

Proxy::Proxy(IEventBus& eventBus, int port)
	: _server(*this, eventBus)
	, _eventBus(eventBus)
	, _local(port)
{
	proxyToLocal();
}

Proxy::~Proxy()
{
	shutdown();
}

void Proxy::shutdown()
{
	reset();
	_server.shutdown();
}

int Proxy::getLocalPort() const
{
	return _local;
}

void Proxy::proxyToLocal()
{
	_address = "127.0.0.1";
	_port = _local;

	runCommand("REPLICAOF NO ONE\r\n");;
}
void Proxy::proxyToAddress(const std::string& address, int port)
{
	if(address != _address || _port != port) {
		_address = address;
		_port = port;

		char val[20] = {0};
		::snprintf(val, sizeof(val), "%u", port);
		runCommand("REPLICAOF " + address + " " + val + "\r\n");
	}
}

void Proxy::reset()
{
	_eventBus.removeAll(IEventBus::Volatile);
}

bool Proxy::listen(int port)
{
	if(! _server.listen(port)) {
		return false;
	}

	std::cout << "Proxy listening on port " << port << std::endl;

	return true;
}

void Proxy::onAccept(const TcpSocket& client)
{
	TcpSocket sock;
	if(! sock.connect(_address, _port)) {
		return;
	}

	_eventBus.add(client, new Copy(*this, client, sock), IEventBus::Volatile);
	_eventBus.add(sock, new Copy(*this, sock, client), IEventBus::Volatile);
}

void Proxy::copy(TcpSocket& from, TcpSocket& to)
{
	char buf[1024];
	if(from.read(buf, sizeof(buf))) {
		if(from.bytes() == 0) {
			_eventBus.remove(from);
			_eventBus.remove(to);
		} else {
			to.write(buf, from.bytes());
		}
	}
}

std::string Proxy::runCommand(const std::string& command)
{
	struct sockaddr_in redis;

	::inet_aton("127.0.0.1", &redis.sin_addr);
	redis.sin_family = AF_INET; 
	redis.sin_port = htons(_local); 

	int sock = ::socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, 0);

	if(::connect(sock, (struct sockaddr*)&redis, sizeof(redis)) != 0) {
		::close(sock);
		return "connect error";
	}

	ssize_t sent = ::send(sock, command.c_str(), command.size(), 0);
	if((size_t)sent != command.size()) {
		::close(sock);
		return "send error";
	}

	char buf[1024] = {0};
	ssize_t bytes = ::recv(sock, buf, sizeof(buf), 0);

	::close(sock);
	if(bytes > 0) {
		return buf;
	} else {
		return "receive error";
	}
}

