#include "Proxy.h"
#include "ReadyRead.h"

int main()
{
	ReadyRead readyRead;
	Proxy proxy(readyRead);

	proxy.address("127.0.0.1", 6379);
	proxy.listen(5000);

	readyRead.run();
}
