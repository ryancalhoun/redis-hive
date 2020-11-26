#include "Controller.h"
#include "Proxy.h"
#include "EventBus.h"

#include "LocalhostCandidateList.h"

#include <iostream>

int main(int argc, const char* argv[])
{
	if(argc < 4) {
		std::cout << "usage " << argv[0] << " REDIS PROXY CONTROLLER [PEER...]" << std::endl;
		exit(0);
	}

	EventBus eventBus;
	Proxy proxy(eventBus, atoi(argv[1]));
	LocalhostCandidateList candidates(atoi(argv[2]));

	Controller controller(proxy, eventBus, candidates);

	proxy.listen(atoi(argv[2]));
	controller.listen(atoi(argv[3]));

	for(int i = 4; i < argc; ++i) {
		candidates.add(atoi(argv[i]));
	}

	eventBus.run();
}
