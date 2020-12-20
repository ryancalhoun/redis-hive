#pragma once

#include "ICandidateList.h"

class DnsCandidateList : public ICandidateList
{
public:
  DnsCandidateList(const std::string& dnsName, int port);

  const std::string& getSelf() const;
  std::vector<std::string> getCandidates() const;

protected:
  void findSelf();

protected:
  std::string _self;
  std::string _dnsName;
  int _port;
};

