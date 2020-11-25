#include "LocalhostCandidateList.h"
#include <cstdio>
#include <cstring>

void LocalhostCandidateList::add(int port)
{
	char buffer [50] = { 0 };

	::strcpy(buffer, "127.0.0.1:");
    ::snprintf(buffer + 10, sizeof(buffer) - 10, "%d", port);

	_list.push_back(buffer);
}

void LocalhostCandidateList::getCandidates(std::vector<std::string>& list) const
{
	list = _list;
}

