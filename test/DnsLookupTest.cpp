#include "DnsLookupTest.h"
#include "DnsLookup.h"

void DnsLookupTest::testLocalhost()
{
  DnsLookup dns;
  std::vector<std::string> ips;

  assert_true(dns.lookup("localhost", ips));
  assert_equal(1, ips.size());
  assert_equal("127.0.0.1", ips[0]);
}

void DnsLookupTest::testInvalid()
{
  DnsLookup dns;
  std::vector<std::string> ips;

  assert_false(dns.lookup("invalid.", ips));
  assert_equal(0, ips.size());
}

void DnsLookupTest::testWellKnown()
{
  DnsLookup dns;
  std::vector<std::string> ips;

  assert_true(dns.lookup("google.com", ips));
  assert_true(dns.lookup("amazon.com", ips));
  assert_true(dns.lookup("gitlab.com", ips));

  assert_greater_equal(3, ips.size());
}

