#include "cppunit/TestResultCollector.h"
#include "cppunit/ui/text/TestRunner.h"

#include "ApplicationTest.h"
#include "DnsCandidateListTest.h"
#include "DnsLookupTest.h"
#include "LocalhostCandidateListTest.h"
#include "MembershipTest.h"
#include "PacketTest.h"

int main(int argc, const char* argv[])
{
  CppUnit::TextUi::TestRunner runner;
  
  runner.addTest(ApplicationTest::suite());
  runner.addTest(DnsCandidateListTest::suite());
  runner.addTest(DnsLookupTest::suite());
  runner.addTest(LocalhostCandidateListTest::suite());
  runner.addTest(MembershipTest::suite());
  runner.addTest(PacketTest::suite());
  runner.run(argc, argv);
  
  return runner.result().testFailures();
}
