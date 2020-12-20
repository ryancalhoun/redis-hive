#include "DnsCandidateList.h"
#include "DnsLookup.h"
#include "UdpSocket.h"

#include <cstring>

namespace
{
  std::string addr(const std::string& host, int port)
  {
    char buffer [50] = { 0 };

    ::strcpy(buffer, host.c_str());
    buffer[host.size()] = ':';
    ::snprintf(buffer + host.size() + 1, sizeof(buffer) - host.size() - 1, "%d", port);

    return buffer;
  }
}

DnsCandidateList::DnsCandidateList(const std::string& dnsName, int port)
  : _dnsName(dnsName)
  , _port(port)
{
  findSelf();
}

const std::string& DnsCandidateList::getSelf() const
{
  return _self;
}

std::vector<std::string> DnsCandidateList::getCandidates() const
{
  DnsLookup dns;
  std::vector<std::string> ips;

  dns.lookup(_dnsName, ips);

  std::vector<std::string>::iterator it;
  for(it = ips.begin(); it != ips.end(); ++it) {
    *it = ::addr(*it, _port);
  }

  return ips;
}

void DnsCandidateList::findSelf()
{
  UdpSocket sock;
  sock.connect("1.2.3.4", 1234);

  _self = ::addr(sock.socketAddress(), _port);
}

