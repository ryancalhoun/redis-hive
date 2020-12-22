#pragma once

#include <vector>
#include <string>
#include <sstream>

class CallHistory
{
public:
  void call(const std::string& m)
  {
    _calls.push_back(m);
  }
  template <typename A1>
  void call(const std::string& m, const A1& a1)
  {
    std::ostringstream out;
    out << m << "(" << a1 << ")";
    _calls.push_back(out.str());
  }
  template <typename A1, typename A2>
  void call(const std::string& m, const A1& a1, const A2& a2)
  {
    std::ostringstream out;
    out << m << "(" << a1 << "," << a2 << ")";
    _calls.push_back(out.str());
  }

  std::string to_s() const
  {
    std::ostringstream out;
    for(std::vector<std::string>::const_iterator it = _calls.begin(); it != _calls.end(); ++it) {
      if(it != _calls.begin()) {
        out << "|";
      }
      out << *it;
    }

    return out.str();
  }

  void clear()
  {
    _calls.clear();
  }

  std::vector<std::string> _calls;
};

