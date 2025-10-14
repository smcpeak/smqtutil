// sync-wait-test.cc
// Tests for `sync-wait` module.

#include "sync-wait.h"                 // module under test

#include "smbase/exc.h"                // smbase::XAssert
#include "smbase/nonport.h"            // getMilliseconds
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <QTimer>

using namespace smbase;


OPEN_ANONYMOUS_NAMESPACE


// Basic test waiting for a timer to expire, no cancelation.
void test_timer()
{
  long startMS = getMilliseconds();

  TestSynchronousWaiter w;
  QTimer timer;
  timer.setSingleShot(true);
  timer.start(50 /*ms*/);
  EXPECT_TRUE(timer.isActive());

  auto condition = [&]() -> bool
    { return !timer.isActive(); };
  EXPECT_TRUE(w.waitUntil(condition, 0, {}, {}));

  long endMS = getMilliseconds();
  long durationMS = endMS - startMS;
  VPVAL(durationMS);

  // On Linux, I've seen this duration be as low as 48 ms, and I suppose
  // that's because the second clock sample might happen before whatever
  // the underlying clock is ticks, so I've weakened the check
  // significantly, to just 30 ms.
  xassert(durationMS >= 30);
}


// Timer with immediate cancelation.
void test_cancel()
{
  long startMS = getMilliseconds();

  TestSynchronousWaiter w(0 /*cancelCountdown*/);
  QTimer timer;
  timer.setSingleShot(true);
  timer.start(50 /*ms*/);
  EXPECT_TRUE(timer.isActive());

  auto condition = [&]() -> bool
    { return !timer.isActive(); };
  EXPECT_FALSE(w.waitUntil(condition, 0, {}, {}));

  long endMS = getMilliseconds();
  long durationMS = endMS - startMS;
  VPVAL(durationMS);
  xassert(durationMS < 50);
}


// Do two waits, canceling only the second.
void test_secondCancel()
{
  long startMS = getMilliseconds();

  TestSynchronousWaiter w(1 /*cancelCountdown*/);
  QTimer timer;
  timer.setSingleShot(true);
  timer.start(50 /*ms*/);
  EXPECT_TRUE(timer.isActive());

  auto condition = [&]() -> bool
    { return !timer.isActive(); };
  EXPECT_TRUE(w.waitUntil(condition, 0, {}, {}));
  EXPECT_EQ(w.m_waitUntilCount, 1);

  long endMS = getMilliseconds();
  long durationMS = endMS - startMS;
  VPVAL(durationMS);
  xassert(durationMS >= 50);

  startMS = endMS;

  timer.start(50 /*ms*/);
  EXPECT_TRUE(timer.isActive());
  EXPECT_FALSE(w.waitUntil(condition, 0, {}, {}));
  EXPECT_EQ(w.m_waitUntilCount, 2);

  endMS = getMilliseconds();
  durationMS = endMS - startMS;
  VPVAL(durationMS);
  xassert(durationMS < 50);
}


// Condition is true right at the start.
void test_immediate()
{
  TestSynchronousWaiter w;
  auto condition = []() -> bool
    { return true; };
  EXPECT_TRUE(w.waitUntil(condition, 0, {}, {}));
  EXPECT_EQ(w.m_waitUntilCount, 1);
}


void test_disallowWaiting()
{
  TestSynchronousWaiter w;
  w.m_disallowWaiting = true;

  // This would try to wait.
  {
    auto condition = []() -> bool
      { return false; };
    EXPECT_EXN_SUBSTR(w.waitUntil(condition, 0, {}, {}),
      XAssert, "waiting not allowed");
    EXPECT_EQ(w.m_waitUntilCount, 1);
  }

  // But even with that flag, we can call `waitUntil` so long as the
  // condition is immediately true.
  {
    auto condition = []() -> bool
      { return true; };
    EXPECT_TRUE(w.waitUntil(condition, 0, {}, {}));
    EXPECT_EQ(w.m_waitUntilCount, 2);
  }
}


CLOSE_ANONYMOUS_NAMESPACE


void test_sync_wait()
{
  test_timer();
  test_cancel();
  test_secondCancel();
  test_immediate();
  test_disallowWaiting();
}


// EOF
