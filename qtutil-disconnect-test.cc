// qtutil-disconnect-test.cc
// Demonstrate the need for `disconnectSignals` in destructors.

// This replicates the `CommandRunner` scenario described in
// doc/signals-and-dtors.txt.  If the `disconnectSignals` call in
// `~Runner` is removed, then when `~DtorSignalEmitter` emits its signal,
// `Runner::on_errorOccurred` runs and writes to the already-destroyed
// `m_errorMessage`, failing its `m_valid` assertion.  Because the slot
// is `noexcept` (and is called from inside a destructor anyway), that
// failure terminates the test program.

#include "qtutil-disconnect-test.h"    // this module

#include "smqtutil/qtutil.h"           // disconnectSignals

#include "smbase/sm-macros.h"          // OPEN_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ
#include "smbase/xassert.h"            // xassert


OPEN_NAMESPACE(qtutil_disconnect_test)


// ----------------------- ValidityCheckedString -----------------------
ValidityCheckedString::ValidityCheckedString()
  : m_valid(true),
    m_value()
{}


ValidityCheckedString::~ValidityCheckedString()
{
  m_valid = false;
}


void ValidityCheckedString::set(std::string const &value)
{
  xassert(m_valid);
  m_value = value;
}


std::string ValidityCheckedString::get() const
{
  xassert(m_valid);
  return m_value;
}


// ------------------------- DtorSignalEmitter -------------------------
DtorSignalEmitter::DtorSignalEmitter()
  : QObject(nullptr /*parent*/)
{}


DtorSignalEmitter::~DtorSignalEmitter()
{
  emitErrorOccurred();
}


void DtorSignalEmitter::emitErrorOccurred()
{
  Q_EMIT signal_errorOccurred();
}


// ------------------------------- Runner ------------------------------
int Runner::s_errorOccurredCalls = 0;


Runner::Runner()
  : QObject(nullptr /*parent*/),
    m_process(),
    m_errorMessage()
{
  QObject::connect(&m_process, &DtorSignalEmitter::signal_errorOccurred,
                   this, &Runner::on_errorOccurred);
}


Runner::~Runner()
{
  // Removing this call makes the test fail.
  //
  // Technically, without this call, we are exercising C++ undefined
  // behavior, so test failure per se is not guaranteed (but is what
  // happens in practice).
  disconnectSignals(&m_process, this);
}


void Runner::on_errorOccurred() noexcept
{
  ++s_errorOccurredCalls;
  m_errorMessage.set("error occurred");
}


// ------------------------------- tests -------------------------------
// Check that the connection works while the `Runner` is alive, so that
// the destruction test below is meaningful.
void testConnectionWorks()
{
  Runner::s_errorOccurredCalls = 0;

  Runner runner;
  runner.m_process.emitErrorOccurred();
  EXPECT_EQ(Runner::s_errorOccurredCalls, 1);
  EXPECT_EQ(runner.m_errorMessage.get(), "error occurred");

  // Destroying `runner` here must not call the slot again.
}


// Destroy a `Runner`, during which `m_process` emits its signal after
// `m_errorMessage` has already been destroyed.
void testDestruction()
{
  Runner::s_errorOccurredCalls = 0;

  {
    Runner runner;
  }

  // Thanks to `disconnectSignals`, the slot was not called.
  EXPECT_EQ(Runner::s_errorOccurredCalls, 0);
}


CLOSE_NAMESPACE(qtutil_disconnect_test)


// Called from unit-tests.cc.
void test_qtutil_disconnect()
{
  using namespace qtutil_disconnect_test;

  testConnectionWorks();
  testDestruction();
}


// EOF
