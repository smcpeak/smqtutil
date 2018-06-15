// test-qtutil.cc
// Tests for 'qtutil' module.

#include "qtutil.h"                    // module to test

#include <QCoreApplication>
#include <QKeyEvent>
#include <QRect>

#include "sm-iostream.h"               // cout, etc.


void testMouseButtonsToString()
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


void testKeyboardModifiersToString()
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


void testKeyEventToString()
{
  cout << "testKeyEventToString" << endl;

  QKeyEvent ev(QEvent::KeyPress, Qt::Key_B,
               Qt::KeyboardModifiers(Qt::ShiftModifier));
  cout << "ev: " << toString(ev) << endl;
}


int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  testMouseButtonsToString();
  testKeyboardModifiersToString();
  testKeyEventToString();

  cout << "QString: " << toString(qstringb("ab" << 'c')) << endl;
  cout << "QRect: " << toString(QRect(10,20,30,40)) << endl;

  return 0;
}  


// EOF
