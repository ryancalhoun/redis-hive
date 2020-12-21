#include "cppunit/CppUnit.h"

#include "LocalhostCandidateList.h"
#include "MockMembershipHandler.h"
#include "MockProxy.h"
#include "MockTimeMachine.h"

class MembershipTest : public CppUnit::TestFixture
{
public:
  MembershipTest();

  void testDefault();


  MockMembershipHandler _handler;
  MockProxy _proxy;
  LocalhostCandidateList _candidates;
  MockTimeMachine _timeMachine;

  static CppUnit::Test* suite()
  {
    CPPUNIT_DEFINE_SUITE(suite, MembershipTest);
    CPPUNIT_ADD_TEST(suite, testDefault);

    return suite;
  }
};
