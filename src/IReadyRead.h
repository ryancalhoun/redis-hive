#pragma once

class IReadyRead
{
public:
	enum Error
	{
		None = 1,
	};

	class ICallback
	{
	public:
		virtual ~ICallback() {}
		virtual int operator()(int fd) = 0;
	};

	virtual ~IReadyRead() {}

	virtual void add(int fd, ICallback* callback) = 0;

	virtual void remove(int fd) = 0;
	virtual int next() = 0;
};

