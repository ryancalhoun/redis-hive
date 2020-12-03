#include "LocalhostCandidateList.h"
#include <cstdio>
#include <cstring>

namespace
{
  std::string addr(int port)
  {
    char buffer [50] = { 0 };

    ::strcpy(buffer, "127.0.0.1:");
    ::snprintf(buffer + 10, sizeof(buffer) - 10, "%d", port);

    return buffer;
  }
}

LocalhostCandidateList::LocalhostCandidateList(int self)
  : _self(addr(self))
{
}

void LocalhostCandidateList::add(int port)
{
  _list.push_back(addr(port));
}

std::string LocalhostCandidateList::getSelf() const
{
  return _self;
}

std::vector<std::string> LocalhostCandidateList::getCandidates() const
{
  return _list;
}

