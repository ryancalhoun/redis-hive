#include "cppunit/CppUnit.h"

class ApplicationTest : public CppUnit::TestFixture
{
public:
  void testDefault();
  void testController();
  void testProxy();
  void testRedis();
  void testMembership();
  void testArgs();

  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, ApplicationTest);
    CPPUNIT_ADD_TEST(suite, testDefault);
    CPPUNIT_ADD_TEST(suite, testController);
    CPPUNIT_ADD_TEST(suite, testProxy);
    CPPUNIT_ADD_TEST(suite, testRedis);
    CPPUNIT_ADD_TEST(suite, testMembership);
    CPPUNIT_ADD_TEST(suite, testArgs);

    return suite;
  }
};
