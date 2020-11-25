#pragma once

class ICandidateList;
class IProxy;
class IReadyRead;

class Controller
{
public:
	Controller(IProxy& proxy, IReadyRead& readyRead, const ICandidateList& candidates);

	bool listen(int port);

	int accept(int fd);
	int read(int fd);

protected:
	IProxy& _proxy;
	IReadyRead& _readyRead;
	const ICandidateList& _candidates;

	int _server;
};

