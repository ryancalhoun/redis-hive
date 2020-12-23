#pragma once

#include <string>

class LogMessage
{
public:
  LogMessage(const char* msg);
  LogMessage(const std::string& msg);

  const std::string& str() const;

protected:
  std::string _msg;
};

template <class charT, class traits> inline
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& s, const LogMessage& msg)
{
  s << msg.str();
  return s;
}
