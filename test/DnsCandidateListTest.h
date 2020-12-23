#include "cppunit/CppUnit.h"
#include "MockLogger.h"

class DnsCandidateListTest : public CppUnit::TestFixture
{
public:
  void testSelf();
  void testCandidates();

  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, DnsCandidateListTest);
    CPPUNIT_ADD_TEST(suite, testSelf);
    CPPUNIT_ADD_TEST(suite, testCandidates);

    return suite;
  }

  MockLogger _logger;
};
