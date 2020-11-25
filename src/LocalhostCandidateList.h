#pragma once

#include "ICandidateList.h"

class LocalhostCandidateList : public ICandidateList
{
public:
	void add(int port);
	void getCandidates(std::vector<std::string>& list) const;

protected:
	std::vector<std::string> _list;
};
