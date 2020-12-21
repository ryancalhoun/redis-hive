#include "Application.h"
#include "EventBus.h"
#include "Controller.h"
#include "Proxy.h"
#include "LocalhostCandidateList.h"
#include "DnsCandidateList.h"
#include "TimeMachine.h"

#include <iostream>
#include <memory>

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

Application::Application()
  : _controllerPort(3000)
  , _proxyPort(7000)
  , _redisPort(6379)
{}

bool Application::parse(int argc, const char* argv[])
{
  optind = 0;

  int c;
  while((c = ::getopt(argc, const_cast<char* const *>(argv), "hc:p:r:m:o:")) != -1) {
    std::string arg = optarg;
    switch(c) {
      case 'h': 
      break;
      case 'c': 
        _controllerPort = ::atoi(arg.c_str());
      break;
      case 'p': 
        _proxyPort = ::atoi(arg.c_str());
      break;
      case 'r': 
        _redisPort = ::atoi(arg.c_str());
      break;
      case 'm': 
        _membership = arg;
      break;
      case 'o': 
        _args.push_back(arg);
      break;
      default:
        std::cout << "Unknown" << std::endl;
        return false;
    }
  }
  return true;
}

int Application::run()
{
  std::unique_ptr<ICandidateList> candidates(createCandidateList());
  if(!candidates) {
    return 1;
  }

  std::cout << candidates->getSelf() << std::endl;
  std::vector<std::string> list = candidates->getCandidates();
  for(std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
    std::cout << " c = " << *it << std::endl;
  }

  TimeMachine timeMachine;
  EventBus eventBus(timeMachine);
  Proxy proxy(eventBus, _redisPort);
  Controller controller(proxy, eventBus, *candidates, timeMachine);

  if(! proxy.listen(_proxyPort)) {
    return 1;
  }
  if(! controller.listen(_controllerPort)) {
    return 1;
  }

  eventBus.run();

  return 0;
}

ICandidateList* Application::createCandidateList() const
{
  if(_membership == "localhost") {
    LocalhostCandidateList* candidates = new LocalhostCandidateList(_controllerPort);
    std::vector<std::string>::const_iterator it;
    for(it = _args.begin(); it != _args.end(); ++it) {
      candidates->add(::atoi(it->c_str()));
    }

    return candidates;
  } else {
    if(_args.size() != 1) {
      return NULL;
    }

    return new DnsCandidateList(_args[0], _controllerPort);
  }
}

int Application::controllerPort() const
{
  return _controllerPort;
}

int Application::proxyPort() const
{
  return _proxyPort;
}

int Application::redisPort() const
{
  return _redisPort;
}

const std::string& Application::membership() const
{
  return _membership;
}

const std::vector<std::string>& Application::args() const
{
  return _args;
}

