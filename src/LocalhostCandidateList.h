#pragma once

#include "ICandidateList.h"

class LocalhostCandidateList : public ICandidateList
{
public:
  LocalhostCandidateList(int self);

  void add(int port);

  std::string getSelf() const;
  std::vector<std::string> getCandidates() const;

protected:
  std::string _self;
  std::vector<std::string> _list;
};
