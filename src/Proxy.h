#include <string>
#include <set>

class IReadyRead;
struct sockaddr_in;

class Proxy {
public:
	Proxy(IReadyRead& readyRead);

	virtual ~Proxy();

	void address(const std::string& address, int port);
	bool listen(int port);

	void shutdown();
	void reset();

	int accept(int client);
	int copy(int from, int to);
	void close(int fd);

protected:
	IReadyRead& _readyRead;
	int _server;
	struct sockaddr_in& _proxy;
	std::set<int> _sockets;
};

