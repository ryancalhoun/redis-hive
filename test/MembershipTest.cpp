#include "MembershipTest.h"
#include "Membership.h"

MembershipTest::MembershipTest()
  : _candidates(3000)
{}

void MembershipTest::testDefault()
{
  Membership m(_handler, _proxy, _candidates, _timeMachine);
}

