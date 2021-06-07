#include "PacketTest.h"
#include "Packet.h"

void PacketTest::testSerialize()
{
  Packet p;
  p.reason(Packet::Ping);
  p.state(Packet::Following);
  p.self("1.2.3.4:567");
  p.following("5.6.7.8:80");
  p.since(1606953004);
  p.proxy(6719);

  assert_equal("a=1.2.3.4:567|e=ping|s=F|f=5.6.7.8:80|r=6719|t=1606953004", p.serialize());
}

void PacketTest::testSerializeLeader()
{
  Packet p;
  p.reason(Packet::Ping);
  p.state(Packet::Leading);
  p.self("1.2.3.4:567");
  p.following("1.2.3.4:567");
  p.since(1606953004);
  p.proxy(6719);
  p.member("4.4.4.4:4444");
  p.member("5.5.5.5:5555");

  assert_equal("a=1.2.3.4:567|e=ping|s=L|f=1.2.3.4:567|r=6719|t=1606953004|m=4.4.4.4:4444,5.5.5.5:5555", p.serialize());
}

void PacketTest::testParse()
{
  Packet p;
  assert_true(p.parse("a=1.2.3.4:567|e=ping|s=F|f=5.6.7.8:80|r=6719|t=1606953004"));
  assert_equal(Packet::Ping, p.reason());
  assert_equal(Packet::Following, p.state());
  assert_equal("1.2.3.4:567", p.self());
  assert_equal("5.6.7.8:80", p.following());
  assert_equal(1606953004, p.since());
  assert_equal(6719, p.proxy());
}

void PacketTest::testParseLeader()
{
  Packet p;
  assert_true(p.parse("a=1.2.3.4:567|e=ping|s=L|f=1.2.3.4:567|r=6719|t=1606953004|m=4.4.4.4:4444,5.5.5.5:5555"));
  assert_equal(Packet::Ping, p.reason());
  assert_equal(Packet::Leading, p.state());
  assert_equal("1.2.3.4:567", p.self());
  assert_equal("1.2.3.4:567", p.following());
  assert_equal(1606953004, p.since());
  assert_equal(6719, p.proxy());

  assert_equal(2, p.members().size());
  assert_equal("4.4.4.4:4444", p.members()[0]);
  assert_equal("5.5.5.5:5555", p.members()[1]);
}

void PacketTest::testParseConstructor()
{
  Packet p("a=1.2.3.4:567|e=ping|s=L|f=1.2.3.4:567|r=6719|t=1606953004|m=4.4.4.4:4444,5.5.5.5:5555");
  assert_equal(Packet::Ping, p.reason());
  assert_equal(Packet::Leading, p.state());
  assert_equal("1.2.3.4:567", p.self());
  assert_equal("1.2.3.4:567", p.following());
  assert_equal(1606953004, p.since());
  assert_equal(6719, p.proxy());

  assert_equal(2, p.members().size());
  assert_equal("4.4.4.4:4444", p.members()[0]);
  assert_equal("5.5.5.5:5555", p.members()[1]);
}

void PacketTest::testParsePartial()
{
  Packet p;
  assert_true(p.parse("a=127.0.0.1:3003|e=ping|s=P|f=|r=6003|t=1621282895199"));
  assert_equal(Packet::Ping, p.reason());
  assert_equal(Packet::Proposing, p.state());
  assert_equal("127.0.0.1:3003", p.self());
  assert_equal("", p.following());
  assert_equal(1621282895199ull, p.since());
  assert_equal(6003, p.proxy());
}

void PacketTest::testWho()
{
  Packet p;
  assert_true(p.parse("e=who"));
  assert_equal(Packet::Who, p.reason());

  p.following("1.2.3.4:80");
  assert_equal("a=|e=who|s=A|f=1.2.3.4:80|r=0|t=0", p.serialize());
}
