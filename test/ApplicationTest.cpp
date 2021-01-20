#include "ApplicationTest.h"
#include "Application.h"

void ApplicationTest::testDefault()
{
  Application app;
  const char* argv[] = { "app" };

  assert_true(app.parse(sizeof(argv) / sizeof(const char*), argv));

  assert_equal(3000, app.controllerPort());
  assert_equal(7000, app.proxyPort());
  assert_equal(6379, app.redisPort());
  assert_equal("", app.membership());
  assert_equal(0, app.args().size());
}

void ApplicationTest::testController()
{
  Application app;
  const char* argv[] = { "app", "-c3001" };

  assert_true(app.parse(sizeof(argv) / sizeof(const char*), argv));

  assert_equal(3001, app.controllerPort());
}

void ApplicationTest::testProxy()
{
  Application app;
  const char* argv[] = { "app", "-p7001" };

  assert_true(app.parse(sizeof(argv) / sizeof(const char*), argv));

  assert_equal(7001, app.proxyPort());
}

void ApplicationTest::testRedis()
{
  ::setenv("PASSVAR", "PASSWORD", 1);

  Application app;
  const char* argv[] = { "app", "-r6001", "-aPASSVAR" };

  assert_true(app.parse(sizeof(argv) / sizeof(const char*), argv));

  assert_equal(6001, app.redisPort());
  assert_equal("PASSWORD", app.redisAuth());
}

void ApplicationTest::testMembership()
{
  Application app;
  const char* argv[] = { "app", "-msomething" };

  assert_true(app.parse(sizeof(argv) / sizeof(const char*), argv));

  assert_equal("something", app.membership());
}

void ApplicationTest::testArgs()
{
  Application app;
  const char* argv[] = { "app", "-oone", "-otwo", "-othree" };

  assert_true(app.parse(sizeof(argv) / sizeof(const char*), argv));

  assert_equal(3, app.args().size());
  assert_equal("one", app.args()[0]);
  assert_equal("two", app.args()[1]);
  assert_equal("three", app.args()[2]);
}

