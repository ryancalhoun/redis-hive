#include "cppunit/CppUnit.h"

#include "LocalhostCandidateList.h"
#include "MockLogger.h"
#include "MockMembershipHandler.h"
#include "MockProxy.h"
#include "MockTimeMachine.h"

class MembershipTest : public CppUnit::TestFixture
{
public:
  MembershipTest();

  void testAlone();
  void testFollow();
  void testLead();
  void testNewCandidate();


  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, MembershipTest);
    CPPUNIT_ADD_TEST(suite, testAlone);
    CPPUNIT_ADD_TEST(suite, testFollow);
    CPPUNIT_ADD_TEST(suite, testLead);
    CPPUNIT_ADD_TEST(suite, testNewCandidate);

    return suite;
  }

  MockMembershipHandler _handler;
  MockProxy _proxy;
  LocalhostCandidateList _candidates;
  MockTimeMachine _timeMachine;
  MockLogger _logger;
};
