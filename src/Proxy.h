#include <string>
#include <map>

#include <netdb.h> 

class Proxy {
public:
	Proxy();

	virtual ~Proxy();

	void address(const std::string& address, int port);
	bool listen(int port);

	void shutdown();
	void close();

	bool wait();

protected:
	bool connect(int client);

	void wait_on(int fd);

	void disconnect(int fd);

protected:
	int _server;
	int _waiter;
	struct sockaddr_in _proxy;
	std::map<int,int> _connections;
};

