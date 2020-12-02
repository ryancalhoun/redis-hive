#include "cppunit/TestResultCollector.h"
#include "cppunit/ui/text/TestRunner.h"

#include "LocalhostCandidateListTest.h"

int main(int argc, const char* argv[])
{
  CppUnit::TextUi::TestRunner runner;
  
  runner.addTest(LocalhostCandidateListTest::suite());
  runner.run(argc, argv);
  
  return runner.result().testFailures();
}
