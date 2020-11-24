#include "Proxy.h"

#include <cstdio> 
#include <cstdlib> 
#include <cstring> 

#include <iostream>

#include <netinet/in.h> 
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <arpa/inet.h>

Proxy::Proxy()
{
}

Proxy::~Proxy() {
	shutdown();
}

void Proxy::shutdown() {
	close();

	::epoll_ctl(_waiter, EPOLL_CTL_DEL, _server, NULL);
	::close(_server);

	::close(_waiter);
}

void Proxy::address(const std::string& address, int port) {
	::inet_aton(address.c_str(), &_proxy.sin_addr);
	_proxy.sin_family = AF_INET; 
	_proxy.sin_port = htons(port); 
}

void Proxy::close() {
	std::map<int,int>::const_iterator it;
	for(it = _connections.begin(); it != _connections.end(); ++it) {
		disconnect(it->first);
	}
}

bool Proxy::listen(int port) {
	_waiter = ::epoll_create(1);

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

	wait_on(_server);

	std::cout << "Listending on port " << port << std::endl;

	return true;
}

bool Proxy::wait() {
	struct epoll_event ev = { 0 };

	::epoll_wait(_waiter, &ev, 1, -1);

	if(ev.data.fd == _server) {
		struct sockaddr_in client;
		socklen_t len = sizeof(client);

		int sock = ::accept(ev.data.fd, (struct sockaddr*)&client, &len);
		if(sock == -1) {
			return false;
		}

		std::cout << "connection from " << inet_ntoa(client.sin_addr) << std::endl;

		connect(sock);
	} else {
		std::map<int,int>::const_iterator it = _connections.find(ev.data.fd);
		if(it != _connections.end()) {
			char buf[1024];
			ssize_t bytes = ::recv(it->first, buf, sizeof(buf), MSG_DONTWAIT);

			if(bytes == 0) {
				std::cout << "closing " << it->first << ", " << it->second << std::endl;
				disconnect(it->first);
				disconnect(it->second);
			} else if(bytes > 0) {
				ssize_t sent = ::send(it->second, buf, bytes, 0);
			}
		}
	}
	return true;
}

bool Proxy::connect(int client) {

	int sock = ::socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		::close(client);
		return false;
	}

	if(::connect(sock, (struct sockaddr*)&_proxy, sizeof(_proxy)) != 0) {
		std::cout << "connect error " << errno << std::endl;
		::close(client);
		::close(sock);
		return false;
	}

	_connections[client] = sock;
	_connections[sock] = client;

	wait_on(client);
	wait_on(sock);

	return true;
}

void Proxy::wait_on(int fd) {
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLIN|EPOLLPRI;

	errno = 0;
	if(::epoll_ctl(_waiter, EPOLL_CTL_ADD, fd, &ev) != 0 && errno == EEXIST) {
		::epoll_ctl(_waiter, EPOLL_CTL_MOD, fd, &ev);
	}
}

void Proxy::disconnect(int fd) {
	::epoll_ctl(_waiter, EPOLL_CTL_DEL, fd, NULL);
	::close(fd);
	_connections.erase(fd);
}

