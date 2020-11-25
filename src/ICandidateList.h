#pragma once

#include <vector>
#include <string>

class ICandidateList
{
public:
	virtual ~ICandidateList() {}
	virtual void getCandidates(std::vector<std::string>& list) const = 0;
};

