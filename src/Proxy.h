#include <string>

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

protected:
	IReadyRead& _readyRead;
	int _server;
	struct sockaddr_in& _proxy;
};

