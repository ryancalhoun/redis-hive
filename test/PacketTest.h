#include "cppunit/CppUnit.h"

class PacketTest : public CppUnit::TestFixture
{
public:
  void testSerialize();
  void testSerializeLeader();
  void testParse();
  void testParseLeader();

  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, PacketTest);
    CPPUNIT_ADD_TEST(suite, testSerialize);
    CPPUNIT_ADD_TEST(suite, testSerializeLeader);
    CPPUNIT_ADD_TEST(suite, testParse);
    CPPUNIT_ADD_TEST(suite, testParseLeader);

    return suite;
  }
};
