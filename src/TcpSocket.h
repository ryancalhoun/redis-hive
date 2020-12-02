#pragma once

#include <string>

class TcpSocket
{
public:
	TcpSocket();
	TcpSocket(int fd);

	void close();

	operator int() const;
	size_t bytes() const;

	bool listen(int port);
	int accept();

	bool connect(const std::string& host, int port);
	bool connect(const std::string& addr);

	bool read(void* buff, size_t length);
	bool write(const void* buff, size_t length);

protected:
	int _fd;
	int _ec;
	size_t _bytes;
};
