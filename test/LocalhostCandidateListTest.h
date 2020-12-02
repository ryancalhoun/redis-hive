#include "cppunit/CppUnit.h"

class LocalhostCandidateListTest : public CppUnit::TestFixture
{
public:
  void testSelf();
  
  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, LocalhostCandidateListTest);
    CPPUNIT_ADD_TEST(suite, testSelf);
    
    return suite;
  }
};
