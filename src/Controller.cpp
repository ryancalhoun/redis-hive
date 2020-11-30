#include "Controller.h"
#include "ICandidateList.h"
#include "IProxy.h"
#include "IEventBus.h"
#include "Time.h"

#include <cstdio> 
#include <cstdlib> 
#include <cstring> 

#include <algorithm>
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

class Controller::Packet
{
public:
	Packet() {}

	Packet(const std::string& data)
	{
		parse(data);
	}

	std::string gets(char c) const
	{
		std::map<char,std::string>::const_iterator it = _data.find(c);
		if(it == _data.end()) {
			return "";
		} else {
			return it->second;
		}
	}
	unsigned long long getu(char c) const
	{
		return ::strtoull(gets(c).c_str(), NULL, 10);
	}
	void set(char c, const std::string& v)
	{
		_data[c] = v;
	}
	void set(char c, char v)
	{
		set(c, std::string(1, v));
	}
	void set(char c, int v)
	{
		set(c, (unsigned long long)v);
	}
	void set(char c, unsigned long long v)
	{
		char val[50] = {0};
		::snprintf(val, sizeof(val), "%llu", v);
		set(c, val);
	}

	std::string serialize() const
	{
		std::string data;
		std::map<char,std::string>::const_iterator it;
		for(it = _data.begin(); it != _data.end(); ++it) {
			data += std::string(1, it->first) + "=" + it->second + "|";
		}
		return data;
	}
	void parse(const std::string& data)
	{
		size_t begin = 0, end = 0;
		for(; end != std::string::npos; begin = end + 1) {
			end = data.find('|', begin);
			std::string part = data.substr(begin, end-begin);
			if(part.size() > 2) {
				_data[part[0]] = part.substr(2);
			}
		}
	}

protected:
	std::map<char,std::string> _data;
};

Controller::Controller(IProxy& proxy, IEventBus& eventBus, const ICandidateList& candidates)
	: _proxy(proxy)
	, _eventBus(eventBus)
	, _candidates(candidates)
	, _interval(5000)
	, _state(Alone)
	, _since(Time().now())
	, _self(candidates.getSelf())
	
{
}

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

	_eventBus.add(_server, new CB(*this, &Controller::accept, _server));
	_eventBus.every(_interval, new CB(*this, &Controller::ping, 0));

	std::cout << "Controller listening on port " << port << std::endl;

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
	char buf[1024] = {0};
	ssize_t bytes = ::recv(fd, buf, sizeof(buf), MSG_DONTWAIT);

	if(bytes == 0) {
		_eventBus.remove(fd);

	} else if(bytes > 0) {
		Packet received(buf);
		std::string peer = received.gets('a');
		std::string peerFollowing = received.gets('f');
		std::string peerState = received.gets('s');
		unsigned long long peerSince = received.getu('t');

		unsigned long long now = Time().now();

		_members[peer] = now;
		if(peerState == "P") {
			_election[peer] = peerSince;
		} else {
			_election.erase(peer);
		}

		if(_state == Alone || _state == Proposing) {
			if(peerFollowing.size() == 0) {
				propose();
			} else {
				follow(peerFollowing);
			}
		} else if(_state == Following) {
			if(_leader == peerFollowing) {
				if(peerState == "L") {
					int port = received.getu('r');
					size_t c = _leader.find(':');
					_proxy.proxyToAddress(_leader.substr(0, c), port);

					std::string peerMembers = received.gets('m');
					size_t begin = 0, end = 0;
					for(; end != std::string::npos; begin = end + 1) {
						end = peerMembers.find(',', begin);
						std::string member = peerMembers.substr(begin, end - begin);
						if(member.size() > 0) {
							_members[member] = now;
						}
					}
				}
			} else {
				// accept the newer
				if(_since < peerSince) {
					follow(peerFollowing);
				}
			}

		} else if(_state == Leading) {
			if(peerState == "L" && _self != peer) {
				// accept the older
				if(_since > peerSince) {
					follow(peer);
				}
			}
		}

		if(received.gets('e') == "ping") {
			Packet ack;
			packetFor("ack", ack);
			sendTo(fd, ack.serialize());
			_eventBus.remove(fd);
			std::cout << "ACK " << ack.serialize() << std::endl;
		}
	}

	return 0;
}

int Controller::ping(int)
{
	purge();

	Packet packet;
	packetFor("ping", packet);

	unsigned long long now = Time().now();

	if(_state == Alone || _state == Proposing) {
		std::vector<std::string> candidates = _candidates.getCandidates();

		std::sort(candidates.begin(), candidates.end());

		size_t sent = 0;
		std::vector<std::string>::const_iterator it;
		for(it = candidates.begin(); it != candidates.end(); ++it) {
			if(*it != _self) {
				if(sendTo(connectTo(*it), packet.serialize())) {
					++sent;
				}
			}
		}

		if(sent == 0) {
			propose();
		}

		if(_state == Proposing) {
			if((int)(now - _since) > _interval*3) {
				election();
			}
		}

	} else if(_state == Following) {
		if(! sendTo(connectTo(_leader), packet.serialize())) {
			propose();
		}
	}

	return 0;
}

void Controller::follow(const std::string& leader)
{
	std::cout << "Follow " << leader << std::endl;
	if(_leader != leader || _state != Following) {
		_since = Time().now();
	}

	_leader = leader;
	_state = Following;
	_election.clear();
}

void Controller::propose()
{
	std::cout << "Propose" << std::endl;
	if(_state != Proposing) {
		_since = Time().now();
	}
	_leader = "";
	_state = Proposing;
	_election[_self] = _since;
}

void Controller::lead()
{
	std::cout << "Lead" << std::endl;
	if(_state != Leading) {
		_since = Time().now();
		_proxy.proxyToLocal();
	}
	_leader = _self;
	_state = Leading;
	_election.clear();
}

int Controller::connectTo(const std::string& peer) const
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET; 

	size_t c = peer.find(':');
	int port = atoi(peer.substr(c+1).c_str());

	::inet_aton(peer.substr(0, c).c_str(), &addr.sin_addr);
	addr.sin_port = htons(port); 

	int sock = ::socket(AF_INET, SOCK_STREAM, 0);
	if(::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
		return sock;
	} else {
		::close(sock);
		return -1;
	}
}

bool Controller::sendTo(int fd, const std::string& data)
{
	if(fd != -1) {
		ssize_t bytes = ::send(fd, data.c_str(), data.size(), 0);
		if(bytes == (ssize_t)data.size()) {
			_eventBus.add(fd, new CB(*this, &Controller::read, fd));
			return true;
		} else {
			::close(fd);
		}
	}
	return false;
}

void Controller::packetFor(const std::string& reason, Packet& packet) const
{
	const char* states = "APLF";

	packet.set('e', reason);
	packet.set('s', states[_state]);
	packet.set('t', _since);
	packet.set('a', _self);
	packet.set('f', _leader);
	packet.set('r', _proxy.getLocalPort());

	if(_state == Leading) {
		std::string members;
		std::map<std::string,unsigned long long>::const_iterator it;
		for(it = _members.begin(); it != _members.end(); ++it) {
			if(it != _members.begin()) {
				members += ",";
			}
			members += it->first;
		}
		packet.set('m', members);
	}
}

void Controller::purge()
{
	unsigned long long now = Time().now();

	std::map<std::string,unsigned long long>::iterator it;
	for(it = _members.begin(); it != _members.end();) {
		if((int)(now - it->second) > _interval * 5) {
			_members.erase(it++);
		} else {
			++it;
		}
	}
}

void Controller::election()
{
	std::map<std::string,unsigned long long>::const_iterator winner = _election.begin();
	std::map<std::string,unsigned long long>::const_iterator it;
	for(it = _election.begin(); it != _election.end(); ++it) {
		if(it->second < winner->second - 10) {
			winner = it;
		}
	}

	if(winner == _election.end() || winner->first == _self) {
		lead();

		Packet packet;
		packetFor("ping", packet);

		std::map<std::string,unsigned long long>::const_iterator it;
		for(it = _members.begin(); it != _members.end(); ++it) {
			if(it->first != _self) {
				sendTo(connectTo(it->first), packet.serialize());
			}
		}
	}
}

