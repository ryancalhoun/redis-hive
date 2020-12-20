#include "DnsCandidateListTest.h"
#include "DnsCandidateList.h"

void DnsCandidateListTest::testSelf()
{
  DnsCandidateList candidates("localhost", 3000);
  std::string self = candidates.getSelf();

  assert_greater_equal(12u, self.size());
  assert_equal(std::string::npos, self.find_first_not_of("0123456789.:"));
}

void DnsCandidateListTest::testCandidates()
{
  DnsCandidateList candidates("localhost", 3000);
  std::vector<std::string> ips = candidates.getCandidates();
  assert_greater_equal((size_t)1, ips.size());
  assert_equal("127.0.0.1:3000", ips[0]);
}

