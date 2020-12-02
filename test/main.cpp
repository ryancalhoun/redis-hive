#include "cppunit/TestResultCollector.h"
#include "cppunit/ui/text/TestRunner.h"

int main(int argc, const char* argv[])
{
  CppUnit::TextUi::TestRunner runner;
  
//  runner.addTest(MyTestClass::suite());
  runner.run(argc, argv);
  
  return runner.result().testFailures();
}
