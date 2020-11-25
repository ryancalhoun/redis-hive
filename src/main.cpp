#include "Controller.h"
#include "Proxy.h"
#include "EventBus.h"

#include "LocalhostCandidateList.h"

#include <iostream>

int main(int argc, const char* argv[])
{
	EventBus eventBus;
	Proxy proxy(eventBus);
	LocalhostCandidateList candidates;

	Controller controller(proxy, eventBus, candidates);

	if(argc < 3) {
		std::cout << "usage " << argv[0] << " PROXY CONTROLLER [PEER...]" << std::endl;
		exit(0);
	}

	proxy.listen(atoi(argv[1]));
	controller.listen(atoi(argv[2]));

	for(int i = 3; i < argc; ++i) {
		candidates.add(atoi(argv[i]));
	}

	eventBus.run();
}
