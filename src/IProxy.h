#pragma once

#include <string>

class IProxy
{
public:
	virtual ~IProxy() {}

	virtual void address(const std::string& address, int port) = 0;
	virtual void reset() = 0;
};

