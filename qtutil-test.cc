// qtutil-test.cc
// Tests for 'qtutil' and 'qtguiutil' modules.

#include "qtguiutil.h"                 // module to test
#include "qtutil.h"                    // module to test

// smbase
#include "sm-iostream.h"               // cout, etc.
#include "sm-test.h"                   // EXPECT_EQ
#include "strutil.h"                   // hasSubstring

// Qt
#include <QByteArray>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QRect>
#include <QShortcutEvent>


static void testMouseButtonsToString()
{
  cout << "testMouseButtonsToString" << endl;

  Qt::MouseButtons b;
  cout << "empty: " << toString(b) << endl;

  b |= Qt::LeftButton;
  cout << "left: " << toString(b) << endl;

  b |= Qt::RightButton;
  cout << "left and right: " << toString(b) << endl;

  b |= Qt::AllButtons;
  cout << "all: " << toString(b) << endl;
}


static void testKeyboardModifiersToString()
{
  cout << "testKeyboardModifiersToString" << endl;

  Qt::KeyboardModifiers k;
  cout << "empty: " << toString(k) << endl;

  k |= Qt::AltModifier;
  cout << "alt: " << toString(k) << endl;

  k |= Qt::ShiftModifier;
  cout << "alt and shift: " << toString(k) << endl;

  k |= Qt::KeyboardModifier(0x7fffffff);
  cout << "all: " << toString(k) << endl;
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

  try {
    cout << "will throw:" << endl;
    getKeyboardModifierFromString("blah");
    xfailure("should have failed");
  }
  catch (xFormat &x) {
    cout << "as expected: " << x.why() << endl;
  }
}


static void testRTKeySequence(QKeySequence const &kseq)
{
  string keyString(toString(kseq.toString()));
  cout << "keyString: " << quoted(keyString) << endl;

  QKeySequence actual = parseKeySequence(keyString);
  xassert(actual == kseq);
}

static void testInvalidKeySequenceString(
  string const &keys, string const &error)
{
  cout << "testing invalid keys: " << quoted(keys) << endl;
  try {
    parseKeySequence(keys);
    xfailure("should have failed!");
  }
  catch (xFormat &x) {
    if (hasSubstring(x.cond(), error)) {
      cout << "as expected: " << x.cond() << endl;
    }
    else {
      xfailure(stringb("wrong error: " << x.cond()));
    }
  }
}

static void testInvalidKeySequence(QKeySequence const &kseq)
{
  string keyString(toString(kseq.toString()));
  cout << "testing invalid key sequence: " << quoted(keyString) << endl;

  testInvalidKeySequenceString(keyString, "unrecognized");
}


static void testParseKeySequence()
{
  cout << "testParseKeySequence" << endl;

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
    cout << "ev: " << quoted(evString) << endl;
  }

  QKeyEvent *ev2 = getKeyPressEventFromString(evString, ev.text());
  EXPECT_EQ(keysString(*ev2), evString);
  delete ev2;
}

static void testKeyPressEventToString()
{
  cout << "testKeyPressEventToString" << endl;

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

  cout << "Exhaustive..." << endl;
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
  cout << "ev: " << evString << endl;

  QShortcutEvent *ev2 = getShortcutEventFromString(evString);
  xassert(evString == toString(ev2->key().toString()));
  delete ev2;
}

static void testShortcutEventToString()
{
  cout << "testShortcutEventToString" << endl;

  testRTShortcutEvent(QShortcutEvent(Qt::Key_X, 0));
  testRTShortcutEvent(QShortcutEvent(Qt::SHIFT+Qt::Key_Y, 0));
  testRTShortcutEvent(QShortcutEvent(Qt::SHIFT+Qt::CTRL+Qt::Key_Y, 0));
}


static void testPrintQByteArray()
{
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

  try {
    cout << "should throw:" << endl;
    qSizeFromString("x");
    xfailure("should have failed");
  }
  catch (xFormat &x) {
    cout << "as expected: " << x.why() << endl;
  }
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


static void entry(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  xBase::logExceptions = false;

  testMouseButtonsToString();
  testKeyboardModifiersToString();
  testKeyboardModifierToString();
  testParseKeySequence();
  testKeyPressEventToString();
  testShortcutEventToString();
  testPrintQByteArray();
  testQSizeFromString();
  testQObjectPath();

  cout << "QString: " << toString(qstringb("ab" << 'c')) << endl;
  cout << "QRect: " << toString(QRect(10,20,30,40)) << endl;

  cout << "qtutil-test: PASSED" << endl;
}

ARGS_TEST_MAIN


// EOF
