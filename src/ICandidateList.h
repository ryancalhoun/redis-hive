#pragma once

#include <vector>
#include <string>

class ICandidateList
{
public:
  virtual ~ICandidateList() {}
  virtual const std::string& getSelf() const = 0;
  virtual std::vector<std::string> getCandidates() const = 0;
};

