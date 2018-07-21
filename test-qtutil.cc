// test-qtutil.cc
// Tests for 'qtutil' and 'qtguiutil' modules.

#include "qtguiutil.h"                 // module to test
#include "qtutil.h"                    // module to test

#include <QByteArray>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QRect>
#include <QShortcutEvent>

#include "sm-iostream.h"               // cout, etc.


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


static void testRTKeyEvent(QKeyEvent const &ev)
{
  string evString(toString(ev));
  cout << "ev: " << evString << endl;

  QKeyEvent *ev2 = getKeyPressEventFromString(evString);
  xassert(evString == toString(*ev2));
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
              Qt::KeyboardModifiers(Qt::ShiftModifier)));
  testRTKeyEvent(
    QKeyEvent(QEvent::KeyPress, Qt::Key_Delete,
              Qt::KeyboardModifiers(Qt::ShiftModifier | Qt::AltModifier)));
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


int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  testMouseButtonsToString();
  testKeyboardModifiersToString();
  testKeyboardModifierToString();
  testKeyPressEventToString();
  testShortcutEventToString();
  testPrintQByteArray();

  cout << "QString: " << toString(qstringb("ab" << 'c')) << endl;
  cout << "QRect: " << toString(QRect(10,20,30,40)) << endl;

  return 0;
}


// EOF
