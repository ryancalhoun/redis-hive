#pragma once
#include <string>
#include <map>

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

	enum State
	{
		Alone,
		Proposing,
		Leading,
		Following,
	};
protected:
	class Packet;

	void follow(const std::string& leader);
	void propose();
	void lead();

	int connectTo(const std::string& peer) const;
	bool sendTo(int fd, const std::string& data);
	void packetFor(const std::string& reason, Packet& packet) const;

	void purge();
	void election();

protected:
	IProxy& _proxy;
	IEventBus& _eventBus;
	const ICandidateList& _candidates;
	int _interval;

	int _server;

	State _state;
	unsigned long long _since;

	std::string _self;
	std::string _leader;

	std::map<std::string,unsigned long long> _members;
	std::map<std::string,unsigned long long> _election;
};

