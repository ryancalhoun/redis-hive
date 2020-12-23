#include "LogMessage.h"

LogMessage::LogMessage(const char* msg)
  : _msg(msg)
{}

LogMessage::LogMessage(const std::string& msg)
  : _msg(msg)
{}

const std::string& LogMessage::str() const
{
  return _msg;
}

