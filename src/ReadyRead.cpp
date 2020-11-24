#include "ReadyRead.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <cstdlib> 
#include <errno.h>
#include <iostream>

ReadyRead::ReadyRead()
	: _waiter(::epoll_create(1))
{
}

ReadyRead::~ReadyRead()
{
	std::map<int,ICallback*>::const_iterator it;
	for(it = _callback.begin(); it != _callback.end(); ++it) {
		delete it->second;
		_callback.erase(it);
	}
	::close(_waiter);
}

void ReadyRead::add(int fd, ICallback* callback)
{
	struct epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = EPOLLIN|EPOLLPRI;

	errno = 0;
	if(::epoll_ctl(_waiter, EPOLL_CTL_ADD, fd, &ev) != 0 && errno == EEXIST) {
		::epoll_ctl(_waiter, EPOLL_CTL_MOD, fd, &ev);
	}
	_callback[fd] = callback;
}

void ReadyRead::remove(int fd)
{
	::epoll_ctl(_waiter, EPOLL_CTL_DEL, fd, NULL);
	std::map<int,ICallback*>::const_iterator it = _callback.find(fd);
	if(it != _callback.end()) {
		delete it->second;
		_callback.erase(it);
	}
}

void ReadyRead::run()
{
	for(;;) {
		next();
	}
}

int ReadyRead::next()
{
	struct epoll_event ev = { 0 };

	if(::epoll_wait(_waiter, &ev, 1, -1) != -1) {
		std::map<int,ICallback*>::const_iterator it = _callback.find(ev.data.fd);
		if(it != _callback.end()) {
			return (*it->second)(it->first);
		}
	}
	return None;
}

