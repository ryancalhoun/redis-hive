#include "Proxy.h"

int main() {

	Proxy proxy;

	proxy.address("127.0.0.1", 6379);
	proxy.listen(5000);

	while(proxy.wait()) {
	}

}
