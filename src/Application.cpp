#include "Application.h"
#include "EventBus.h"
#include "Controller.h"
#include "Proxy.h"
#include "LocalhostCandidateList.h"
#include "DnsCandidateList.h"
#include "StdoutLogger.h"
#include "TimeMachine.h"

#include <memory>

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

Application::Application()
  : _controllerPort(3000)
  , _proxyPort(7000)
  , _redisPort(6379)
  , _usage(false)
{
  _usageMessage =
  "Usage: redis-hive [OPTIONS]\n"
  "\n"
  "Options:\n"
  "  -cPORT     Controller listen port [3000]\n"
  "  -pPORT     Proxy listen port [7000]\n"
  "  -rPORT     Redis listen port [6379]\n"
  "  -aVAR      Environment variable containing redis auth password\n"
  "  -mMETHOD   Membership method\n"
  "  -oOPTION   Membership options\n"
  "\n"
  "Membership:\n"
  "  The value of METHOD for membership can be one of `localhost' or\n"
  "  `cluster`. If using `localhost`, specify the controller port value\n"
  "  for each hive instance with -o. If using `dns', specify the headless\n"
  "  service name that resolves to the cluster IP of each pod.\n"
  "\n"
  "  Examples:\n"
  "  redis-hive -mlocalhost -o3000 -o3001 -o3002\n"
  "  redis-hive -mcluster -oredis-hive-headless\n"
  ;
}

namespace
{
  std::string safenull(const char* p)
  {
    return p ? p : "";
  }
}

bool Application::parse(int argc, const char* argv[])
{
  optind = 0;

  int c;
  while((c = ::getopt(argc, const_cast<char* const *>(argv), "hc:p:r:a:m:o:")) != -1) {
    std::string arg;
    if(optarg) {
      arg = optarg;
    }
    switch(c) {
      case 'h': 
        _usage = true;
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
      case 'a': 
        _redisAuth = ::safenull(::getenv(arg.c_str()));
      break;
      case 'm': 
        _membership = arg;
      break;
      case 'o': 
        _args.push_back(arg);
      break;
      default:
    //    _errorMessage = "Unknown option '" + std::string(1, c) + "'";
        _errorMessage = opterr;
        return false;
    }
  }
  return true;
}

bool Application::usage() const
{
  return _usage;
}

const std::string& Application::usageMessage() const
{
  return _usageMessage;
}

const std::string& Application::errorMessage() const
{
  return _errorMessage;
}

int Application::run()
{
  TimeMachine timeMachine;
  StdoutLogger logger(timeMachine);

  std::unique_ptr<ICandidateList> candidates(createCandidateList(logger));
  if(!candidates) {
    return 1;
  }

  EventBus eventBus(timeMachine, logger);
  Proxy proxy(eventBus, _redisPort, _redisAuth, logger);
  Controller controller(proxy, eventBus, *candidates, timeMachine, logger);

  if(! proxy.listen(_proxyPort)) {
    return 1;
  }
  if(! controller.listen(_controllerPort)) {
    return 1;
  }

  eventBus.run();

  return 0;
}

ICandidateList* Application::createCandidateList(ILogger& logger) const
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

    return new DnsCandidateList(_args[0], _controllerPort, logger);
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

const std::string& Application::redisAuth() const
{
  return _redisAuth;
}

const std::string& Application::membership() const
{
  return _membership;
}

const std::vector<std::string>& Application::args() const
{
  return _args;
}

