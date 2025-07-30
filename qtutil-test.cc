// qtutil-test.cc
// Tests for 'qtutil' and 'qtguiutil' modules.

#include "qtutil-test.h"               // this module

#include "qtutil.h"                    // module to test
#include "qtguiutil.h"                 // module to test

// smbase
#include "smbase/exc.h"                // smbase::XFormat
#include "smbase/sm-iostream.h"        // cout, etc.
#include "smbase/sm-test.h"            // DIAG, EXPECT_EQ
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/strutil.h"            // hasSubstring
#include "smbase/xassert.h"            // xfailure_stringbc

// Qt
#include <QByteArray>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QRect>
#include <QShortcutEvent>

// libc++
#include <string_view>                 // std::string_view


using namespace smbase;


OPEN_NAMESPACE(qtutil_test)


// ------------------------------- Sender ------------------------------
Sender::Sender()
  : QObject(nullptr /*parent*/)
{}


Sender::~Sender()
{}


// ------------------------------ Receiver -----------------------------
Receiver::Receiver()
  : QObject(nullptr /*parent*/),
    m_receipts(0)
{}


Receiver::~Receiver()
{
  disconnectSignalSender(this);
}


void Receiver::on_sig1() noexcept
{
  m_receipts++;
}


// ------------------------------- tests -------------------------------
static void testMouseButtonsToString()
{
  DIAG("testMouseButtonsToString");

  Qt::MouseButtons b;
  EXPECT_EQ(toString(b), "NoButton");

  b |= Qt::LeftButton;
  EXPECT_EQ(toString(b), "LeftButton");

  b |= Qt::RightButton;
  EXPECT_EQ(toString(b), "LeftButton+RightButton");

  b |= Qt::AllButtons;
  EXPECT_EQ(toString(b),
    "LeftButton+"
    "RightButton+"
    "MiddleButton+"
    "BackButton+"
    "ForwardButton+"
    "TaskButton+"
    "ExtraButton4+"
    "ExtraButton5 "
    "(plus unknown flags: 134217472)");
}


static void testKeyboardModifiersToString()
{
  DIAG("testKeyboardModifiersToString");

  Qt::KeyboardModifiers k;
  EXPECT_EQ(toString(k), "NoModifier");

  k |= Qt::AltModifier;
  EXPECT_EQ(toString(k), "Alt");

  k |= Qt::ShiftModifier;
  EXPECT_EQ(toString(k), "Shift+Alt");

  k |= Qt::KeyboardModifier(0x7fffffff);
  EXPECT_EQ(toString(k),
    "Shift+"
    "Ctrl+"
    "Alt+"
    "Meta+"
    "Keypad+"
    "GroupSwitch "
    "(plus unknown flags: 33554431)");
}


static void testRTKeyboardModifier(Qt::KeyboardModifier m)
{
  string s = toString(Qt::KeyboardModifiers(m));
  Qt::KeyboardModifier m2 = getKeyboardModifierFromString(s);
  xassert(m == m2);
}

static void testKeyboardModifierToString()
{
  testRTKeyboardModifier(Qt::NoModifier);
  testRTKeyboardModifier(Qt::ShiftModifier);
  testRTKeyboardModifier(Qt::ControlModifier);
  testRTKeyboardModifier(Qt::AltModifier);
  testRTKeyboardModifier(Qt::MetaModifier);
  testRTKeyboardModifier(Qt::KeypadModifier);
  testRTKeyboardModifier(Qt::GroupSwitchModifier);

  EXPECT_EXN(getKeyboardModifierFromString("blah"), XFormat);
}


static void testRTKeySequence(QKeySequence const &kseq)
{
  string keyString(toString(kseq.toString()));
  DIAG("keyString: " << doubleQuote(keyString));

  QKeySequence actual = parseKeySequence(keyString);
  xassert(actual == kseq);
}

static void testInvalidKeySequenceString(
  string const &keys, string const &error)
{
  DIAG("testing invalid keys: " << doubleQuote(keys));
  EXPECT_EXN_SUBSTR(parseKeySequence(keys),
    XFormat, error.c_str());
}

static void testInvalidKeySequence(QKeySequence const &kseq)
{
  string keyString(toString(kseq.toString()));
  DIAG("testing invalid key sequence: " << doubleQuote(keyString));

  testInvalidKeySequenceString(keyString, "unrecognized");
}


static void testParseKeySequence()
{
  DIAG("testParseKeySequence");

  testRTKeySequence(QKeySequence(
    Qt::Key_A));
  testRTKeySequence(QKeySequence(
    Qt::SHIFT | Qt::Key_Plus));
  testRTKeySequence(QKeySequence(
    Qt::SHIFT | Qt::CTRL | Qt::Key_Comma));
  testRTKeySequence(QKeySequence(
    Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::Key_Period));
  testRTKeySequence(QKeySequence(
    Qt::Key_Semicolon));
  testRTKeySequence(QKeySequence(
    Qt::Key_Semicolon, Qt::Key_Semicolon));
  testRTKeySequence(QKeySequence(
    Qt::SHIFT | Qt::Key_Semicolon, Qt::CTRL | Qt::Key_Semicolon));
  testRTKeySequence(QKeySequence(
    Qt::Key_Comma, Qt::Key_Semicolon, Qt::Key_Comma, Qt::Key_Semicolon));

  testInvalidKeySequenceString("", "no keys");
  testInvalidKeySequenceString("frog", "unrecognized");
  testInvalidKeySequenceString("Shift+Ctrl+A", "canonical");

  // QKeySequence cannot represent bare modifiers.  The stringified form
  // ("\xE1\x9F\x80?") is really weird ("U+17C0 KHMER VOWEL SIGN IE"
  // followed by "?") and doesn't decode.
  testInvalidKeySequence(QKeySequence(
    Qt::Key_Shift));

  // These variations also do not work.
  testInvalidKeySequence(QKeySequence(
    Qt::SHIFT | Qt::Key_Shift));
  testInvalidKeySequence(QKeySequence(
    Qt::SHIFT));
  testInvalidKeySequence(QKeySequence(
    Qt::CTRL | Qt::SHIFT | Qt::Key_Shift));

  // These forms do not decode.
  testInvalidKeySequenceString("Shift", "unrecognized");
  testInvalidKeySequenceString("Shift+Shift", "unrecognized");
  testInvalidKeySequenceString("Ctrl+Shift", "unrecognized");
}


static void testRTKeyEvent(QKeyEvent const &ev, bool quiet=false)
{
  string evString(keysString(ev));
  if (!quiet) {
    DIAG("ev: " << doubleQuote(evString));
  }

  QKeyEvent *ev2 = getKeyPressEventFromString(evString, ev.text());
  EXPECT_EQ(keysString(*ev2), evString);
  delete ev2;
}

static void testKeyPressEventToString()
{
  DIAG("testKeyPressEventToString");

  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_Escape,
              Qt::KeyboardModifiers(Qt::NoModifier)));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_B,
              Qt::KeyboardModifiers(Qt::ShiftModifier), "b"));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_X,
              Qt::KeyboardModifiers(Qt::NoModifier), "x"));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_X,
              Qt::KeyboardModifiers(Qt::ControlModifier), "x"));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_Delete,
              Qt::KeyboardModifiers(Qt::ShiftModifier | Qt::AltModifier)));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_Shift,
              Qt::KeyboardModifiers(Qt::ShiftModifier)));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_Shift,
              Qt::KeyboardModifiers(Qt::NoModifier)));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_Shift,
              Qt::KeyboardModifiers(Qt::ControlModifier)));

  DIAG("Exhaustive...");
  for (int keyIndex=0; keyIndex < g_qtKeyNames.m_size; keyIndex++) {
    Qt::Key key = g_qtKeyNames.m_names[keyIndex].m_value;
    for (int modsIndex=0; modsIndex < 8; modsIndex++) {
      Qt::KeyboardModifiers mods = Qt::NoModifier;
      if (modsIndex & 1) {
        mods |= Qt::ShiftModifier;
      }
      if (modsIndex & 2) {
        mods |= Qt::ControlModifier;
      }
      if (modsIndex & 4) {
        mods |= Qt::AltModifier;
      }
      testRTKeyEvent(QKeyEvent(QEvent::KeyPress, key, mods),
                     true /*quiet*/);
    }
  }
}


static void testRTShortcutEvent(QShortcutEvent const &ev)
{
  string evString(toString(ev.key().toString()));
  DIAG("ev: " << evString);

  QShortcutEvent *ev2 = getShortcutEventFromString(evString);
  xassert(evString == toString(ev2->key().toString()));
  delete ev2;
}

static void testShortcutEventToString()
{
  DIAG("testShortcutEventToString");

  testRTShortcutEvent(QShortcutEvent(Qt::Key_X, 0));
  testRTShortcutEvent(QShortcutEvent(Qt::SHIFT+Qt::Key_Y, 0));
  testRTShortcutEvent(QShortcutEvent(Qt::SHIFT+Qt::CTRL+Qt::Key_Y, 0));
}


static void testPrintQByteArray()
{
  if (!verbose) {
    // The function being tested cannot write to an arbitrary stream, so
    // I can't use `EXPECT_EQ`.  It's also not very important.  So, just
    // skip this test if we are not in verbose mode.
    return;
  }

  QByteArray ba;
  printQByteArray(ba, "empty");

  for (int i=0; i < 256; i++) {
    ba.append((char)i);
  }
  printQByteArray(ba, "allchars");
}


static void testRTQSizeFromString(QSize const &size)
{
  string str(toString(size));
  QSize size2(qSizeFromString(str));
  xassert(size == size2);
}

static void testQSizeFromString()
{
  testRTQSizeFromString(QSize(0,0));
  testRTQSizeFromString(QSize(3,4));
  testRTQSizeFromString(QSize(1234567890,1029384756));

  EXPECT_EXN(qSizeFromString("x"), XFormat);
}


static void expectQObjectPath(QObject const *obj, string const &expect)
{
  string actual = qObjectPath(obj);
  EXPECT_EQ(actual, expect);
}

static void testQObjectPath()
{
  QObject root;
  root.setObjectName("root");
  expectQObjectPath(&root, "root");

  QObject *child1 = new QObject(&root);
  child1->setObjectName("child1");

  QObject *child2 = new QObject(&root);
  child2->setObjectName("child2");

  QObject *child3 = new QObject(&root);
  // No name assigned.

  QObject *gc1 = new QObject(child2);
  gc1->setObjectName("gc1");

  QObject *gc2 = new QObject(child3);
  gc2->setObjectName("gc2");

  expectQObjectPath(child1, "root.child1");
  expectQObjectPath(child2, "root.child2");
  expectQObjectPath(gc1, "root.child2.gc1");
  expectQObjectPath(child3, "root.#2");
  expectQObjectPath(gc2, "root.#2.gc2");
}


static void testDisconnectSignals()
{
  Sender s;
  Receiver r1;
  Receiver r2;

  QObject::connect(&s, &Sender::signal_sig1,
                   &r1, &Receiver::on_sig1);
  QObject::connect(&s, &Sender::signal_sig1,
                   &r2, &Receiver::on_sig1);

  xassert(r1.m_receipts == 0);
  xassert(r2.m_receipts == 0);

  Q_EMIT s.signal_sig1();

  xassert(r1.m_receipts == 1);
  xassert(r2.m_receipts == 1);

  disconnectSignalSender(&s);

  Q_EMIT s.signal_sig1();

  // The counts are unchanged because the signals are disconnected.
  xassert(r1.m_receipts == 1);
  xassert(r2.m_receipts == 1);
}


// Check that we can round-trip between `std::string` and `QString`,
// including preserving embedded NULs.
static void testStringConversion()
{
  std::string s1("a\0b", 3);
  EXPECT_EQ(s1.size(), 3);

  QString s2 = toQString(s1);
  EXPECT_EQ(s2.length(), 3);

  std::string s3 = toString(s2);
  EXPECT_EQ(s3, s1);
  EXPECT_EQ(s1.size(), 3);

  std::string_view sv(s3);
  EXPECT_EQ(toQString(sv).length(), 3);

  char const *cstr = s1.c_str();
  EXPECT_EQ(toQString(cstr).length(), 1);   // gets truncated
}


CLOSE_NAMESPACE(qtutil_test)


// Called from unit-tests.cc.
void test_qtutil()
{
  using namespace qtutil_test;

  testMouseButtonsToString();
  testKeyboardModifiersToString();
  testKeyboardModifierToString();
  testParseKeySequence();
  testKeyPressEventToString();
  testShortcutEventToString();
  testPrintQByteArray();
  testQSizeFromString();
  testQObjectPath();
  testDisconnectSignals();
  testStringConversion();

  DIAG("QString: " << toString(qstringb("ab" << 'c')));
  DIAG("QRect: " << toString(QRect(10,20,30,40)));

  DIAG("qtutil-test: PASSED");
}


// EOF
