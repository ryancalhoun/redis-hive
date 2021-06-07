#include "cppunit/CppUnit.h"

class PacketTest : public CppUnit::TestFixture
{
public:
  void testSerialize();
  void testSerializeLeader();
  void testParse();
  void testParseLeader();
  void testParseConstructor();
  void testParsePartial();

  void testWho();

  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, PacketTest);
    CPPUNIT_ADD_TEST(suite, testSerialize);
    CPPUNIT_ADD_TEST(suite, testSerializeLeader);
    CPPUNIT_ADD_TEST(suite, testParse);
    CPPUNIT_ADD_TEST(suite, testParseLeader);
    CPPUNIT_ADD_TEST(suite, testParseConstructor);
    CPPUNIT_ADD_TEST(suite, testParsePartial);

    CPPUNIT_ADD_TEST(suite, testWho);

    return suite;
  }
};
