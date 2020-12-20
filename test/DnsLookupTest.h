#include "cppunit/CppUnit.h"

class DnsLookupTest : public CppUnit::TestFixture
{
public:
  void testLocalhost();
  void testInvalid();
  void testWellKnown();

  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, DnsLookupTest);
    CPPUNIT_ADD_TEST(suite, testLocalhost);
    CPPUNIT_ADD_TEST(suite, testInvalid);
    CPPUNIT_ADD_TEST(suite, testWellKnown);

    return suite;
  }
};
