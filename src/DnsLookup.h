#pragma once

#include <string>
#include <vector>

class DnsLookup
{
public:
  bool lookup(const std::string& name, std::vector<std::string>& ips) const;
};

