#include "Controller.h"
#include "ICandidateList.h"
#include "IProxy.h"
#include "IEventBus.h"
#include "Time.h"
#include "TcpSocket.h"

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
	class Ping : public IEventBus::ICallback
	{
	public:
		Ping(Controller& controller)
			: _controller(controller)
		{}

		int operator()()
		{
			return _controller.ping();
		}
	protected:
		Controller& _controller;
	};

	class Read : public IEventBus::ICallback
	{
	public:
		Read(Controller& controller, const TcpSocket& client)
			: _controller(controller)
			, _client(client)
		{}

		~Read()
		{
			_client.close();
		}

		int operator()()
		{
			return _controller.read(_client);
		}
	protected:
		Controller& _controller;
		TcpSocket _client;
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
	: _server(*this, eventBus)
	, _proxy(proxy)
	, _eventBus(eventBus)
	, _candidates(candidates)
	, _interval(5000)
	, _state(Alone)
	, _since(Time().now())
	, _self(candidates.getSelf())
	
{
	TcpSocket sock;
}

void Controller::onAccept(const TcpSocket& client)
{
	_eventBus.add(client, new Read(*this, client));
}

bool Controller::listen(int port)
{
	if(! _server.listen(port)) {
		return false;
	}

	_eventBus.every(_interval, new Ping(*this));

	std::cout << "Controller listening on port " << port << std::endl;

	return true;
}

int Controller::read(TcpSocket& client)
{
	char buf[1024] = {0};
	if(! client.read(buf, sizeof(buf))) {
		return -1;
	}

	if(client.bytes() == 0) {
		_eventBus.remove(client);
	} else {
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
			std::string reply = ack.serialize();
			client.write(reply.c_str(), reply.size());
			_eventBus.remove(client);
			std::cout << "ACK " << reply << std::endl;
		}
	}

	return 0;
}

int Controller::ping()
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
				if(sendTo(*it, packet.serialize())) {
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
		if(! sendTo(_leader, packet.serialize())) {
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

bool Controller::sendTo(const std::string& addr, const std::string& data)
{
	TcpSocket sock;
	if(! sock.connect(addr) || ! sock.write(data.c_str(), data.size())) {
		sock.close();
		return false;
	}

	_eventBus.add(sock, new Read(*this, sock));
	return true;
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
				sendTo(it->first, packet.serialize());
			}
		}
	}
}

