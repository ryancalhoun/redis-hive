#include "MembershipTest.h"
#include "Membership.h"

MembershipTest::MembershipTest()
  : _candidates(3000)
{
  _candidates.add(3000);
}

void MembershipTest::testAlone()
{
  Membership m(_handler, _proxy, _candidates, _timeMachine, _logger);

  m.onTimer();

  assert_equal("", _handler._calls.to_s());
  assert_equal("proxyToLocal", _proxy._calls.to_s());
}

void MembershipTest::testFollow()
{
  Membership m(_handler, _proxy, _candidates, _timeMachine, _logger);

  _timeMachine._now = 0;

  Packet follower;
  follower.reason(Packet::Ack);
  follower.state(Packet::Following);
  follower.following("1.2.3.4:3001");

  m.onRead(follower);

  assert_equal("", _proxy._calls.to_s());


  Packet leader;
  leader.reason(Packet::Ack);
  leader.state(Packet::Leading);
  leader.following("1.2.3.4:3001");
  leader.proxy(7001);

  m.onRead(leader);

  assert_equal("proxyToAddress(1.2.3.4,7001)", _proxy._calls.to_s());
}

void MembershipTest::testLead()
{
  _timeMachine._now = 1000;
  Membership m(_handler, _proxy, _candidates, _timeMachine, _logger);

  _candidates.add(3001);

  m.onTimer();
  assert_equal("ping(127.0.0.1:3001,a=127.0.0.1:3000|e=ping|s=A|f=|r=6000|t=1000)", _handler._calls.to_s());

  Packet peer;
  peer.reason(Packet::Ack);
  peer.state(Packet::Proposing);
  peer.self("127.0.0.1:3001");
  peer.since(2000);

  m.onRead(peer);

  assert_equal("proxyToLocal", _proxy._calls.to_s());
}

void MembershipTest::testNewCandidate()
{
  _timeMachine._now = 1000;
  Membership m(_handler, _proxy, _candidates, _timeMachine, _logger);

  m.onTimer();

  assert_equal("", _handler._calls.to_s());
  assert_equal("proxyToLocal", _proxy._calls.to_s());

  _candidates.add(3001);

  m.onTimer();
  assert_equal("ping(127.0.0.1:3001,a=127.0.0.1:3000|e=ping|s=L|f=127.0.0.1:3000|r=6000|t=1000)", _handler._calls.to_s());
}

