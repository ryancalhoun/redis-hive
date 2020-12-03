#include "cppunit/TestResultCollector.h"
#include "cppunit/ui/text/TestRunner.h"

#include "LocalhostCandidateListTest.h"
#include "PacketTest.h"

int main(int argc, const char* argv[])
{
  CppUnit::TextUi::TestRunner runner;
  
  runner.addTest(LocalhostCandidateListTest::suite());
  runner.addTest(PacketTest::suite());
  runner.run(argc, argv);
  
  return runner.result().testFailures();
}
