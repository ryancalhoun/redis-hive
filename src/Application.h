#pragma once

#include <string>
#include <vector>

class ICandidateList;
class ILogger;

class Application
{
public:
  Application();

  bool parse(int argc, const char* argv[]);

  int run();

  int controllerPort() const;
  int proxyPort() const;
  int redisPort() const;
  const std::string& redisAuth() const;
  const std::string& membership() const;
  const std::vector<std::string>& args() const;

protected:
  ICandidateList* createCandidateList(ILogger& logger) const;

protected:
  int _controllerPort;
  int _proxyPort;
  int _redisPort;
  std::string _redisAuth;

  std::string _membership;
  std::vector<std::string> _args;
};

