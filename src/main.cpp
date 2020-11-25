#include "Controller.h"
#include "Proxy.h"
#include "EventBus.h"

#include "LocalhostCandidateList.h"

#include <iostream>

int main(int argc, const char* argv[])
{
	if(argc < 3) {
		std::cout << "usage " << argv[0] << " PROXY CONTROLLER [PEER...]" << std::endl;
		exit(0);
	}

	EventBus eventBus;
	Proxy proxy(eventBus);
	LocalhostCandidateList candidates(atoi(argv[2]));

	Controller controller(proxy, eventBus, candidates);

	proxy.listen(atoi(argv[1]));
	controller.listen(atoi(argv[2]));

	for(int i = 3; i < argc; ++i) {
		candidates.add(atoi(argv[i]));
	}

	eventBus.run();
}
