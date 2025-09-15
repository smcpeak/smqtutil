// gdvalue-qt-test.cc
// Tests for `gdvalue-qt` module.

#include "gdvalue-qt.h"                          // module under test

#include "smbase/gdvalue-parser.h"               // gdv::{GDValueParser, gdvpTo}
#include "smbase/gdvalue.h"                      // gdv::toGDValue
#include "smbase/gdvn-test-roundtrip.h"          // gdvnTestRoundtrip
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // EXPECT_EQ

#include <QMouseEvent>
#include <QPoint>
#include <QRect>
#include <QString>

using namespace gdv;


OPEN_ANONYMOUS_NAMESPACE


void test_QPoint()
{
  EXPECT_EQ(toGDValue(QPoint(4,5)).asString(), "QPoint[x:4 y:5]");
}


void test_QPointF()
{
  EXPECT_EQ(toGDValue(QPointF(4.5,5.5)).asString(),
            "QPointF[x:4.5 y:5.5]");
}


void test_QRect()
{
  EXPECT_EQ(toGDValue(QRect(4,5,6,7)).asString(),
    "QRect[left:4 top:5 width:6 height:7]");
}


void test_QString()
{
  EXPECT_EQ(toGDValue(QString("abc")).asString(), "\"abc\"");
}


void test_QSize()
{
  gdvnTestRoundtripEq(QSize(4,5), "QSize(4 5)");

  EXPECT_EQ(toGDValue(QSize(4,5)).asString(), "QSize(4 5)");
}


// TODO: Move to `gdvalue-parser.h`.
template <typename T>
T fromGDVNTo(char const *gdvn)
{
  return gdvpTo<T>(GDValueParser(fromGDVN(gdvn)));
}


void test_QEvent_Type()
{
  gdvnTestRoundtripEq(QEvent::MouseButtonRelease,
    "MouseButtonRelease");
  gdvnTestRoundtripEq(QEvent::User,
    "UserEvent(1000)");

  EXPECT_EXN_SUBSTR(fromGDVNTo<QEvent::Type>("SomeSymbol"),
    XGDValueError,
    "Unrecognized QEvent::Type: `SomeSymbol`.");

  EXPECT_EXN_SUBSTR(fromGDVNTo<QEvent::Type>("UserEvent()"),
    XGDValueError,
    "Expected container to have 1 elements, but it instead has 0 elements.");

  EXPECT_EXN_SUBSTR(fromGDVNTo<QEvent::Type>("UserEvent[]"),
    XGDValueError,
    "Expected tagged tuple, not tagged sequence.");
}


void test_KeyboardModifier()
{
  TEST_CASE(__func__);

  EXPECT_EQ_GDV(Qt::AltModifier,
    fromGDVN("Alt"));

  gdvnTestRoundtripEq(Qt::AltModifier,
    "Alt");
  gdvnTestRoundtripEq(Qt::KeyboardModifier(456),
    "UnknownKeyboardModifier(456)");

  EXPECT_EXN_SUBSTR(fromGDVNTo<Qt::KeyboardModifier>("SomeSymbol"),
    XGDValueError,
    "Unrecognized Qt::KeyboardModifier: `SomeSymbol`.");
}


void test_MouseButton()
{
  gdvnTestRoundtripEq(Qt::MiddleButton,
    "MiddleButton");
  gdvnTestRoundtripEq(Qt::MouseButton(123),
    "UnknownMouseButton(123)");

  EXPECT_EXN_SUBSTR(fromGDVNTo<Qt::MouseButton>("SomeSymbol"),
    XGDValueError,
    "Unrecognized Qt::MouseButton: `SomeSymbol`.");
}


void test_KeyboardModifiers()
{
  gdvnTestRoundtripEq(Qt::KeyboardModifiers(Qt::NoModifier),
    "{}");

  gdvnTestRoundtripEq(Qt::KeyboardModifiers(Qt::AltModifier),
    "{Alt}");

  gdvnTestRoundtripEq(
    Qt::KeyboardModifiers(Qt::AltModifier | Qt::ShiftModifier),
    "{Alt Shift}");

  gdvnTestRoundtripEq(
    Qt::KeyboardModifiers(Qt::ShiftModifier | Qt::KeyboardModifier(2)),
    "{Shift UnknownKeyboardModifier(2)}");

  EXPECT_EXN_SUBSTR(fromGDVNTo<Qt::KeyboardModifiers>("{SomeSymbol}"),
    XGDValueError,
    "Unrecognized Qt::KeyboardModifier: `SomeSymbol`.");

  EXPECT_EXN_SUBSTR(fromGDVNTo<Qt::KeyboardModifiers>("SomeSymbol"),
    XGDValueError,
    "Expected set, not symbol.");
}


void test_MouseButtons()
{
  gdvnTestRoundtripEq(Qt::MouseButtons(Qt::NoButton),
    "{}");

  gdvnTestRoundtripEq(Qt::MouseButtons(Qt::LeftButton),
    "{LeftButton}");

  gdvnTestRoundtripEq(
    Qt::MouseButtons(Qt::LeftButton | Qt::RightButton),
    "{LeftButton RightButton}");

  // 0x1000 is actually `ExtraButton10`, but my table does not include
  // it, so it exercises the "unknown" case.
  gdvnTestRoundtripEq(
    Qt::MouseButtons(Qt::MiddleButton | Qt::MouseButton(0x1000)),
    "{MiddleButton UnknownMouseButton(4096)}");

  EXPECT_EXN_SUBSTR(fromGDVNTo<Qt::MouseButtons>("{SomeSymbol}"),
    XGDValueError,
    "Unrecognized Qt::MouseButton: `SomeSymbol`.");
}


void test_QMouseEvent()
{
  {
    QMouseEvent ev(
      QEvent::MouseButtonPress,
      QPointF(1, 2),                   // local
      QPointF(3, 4),                   // window
      QPointF(5, 6),                   // screen
      Qt::LeftButton,
      Qt::LeftButton | Qt::RightButton,
      Qt::ShiftModifier | Qt::ControlModifier);

    EXPECT_EQ_GDV(toGDValue(ev), fromGDVN(R"(
      QMouseEvent[
        type: MouseButtonPress
        spontaneous: false
        isAccepted: true               // Evidently!
        modifiers: {Shift Ctrl}
        timestamp: 0
        button: LeftButton
        buttons: {LeftButton, RightButton}
        pos: QPoint[x:1 y:2]
        windowPos: QPointF[x:3.0 y:4.0]
        globalPos: QPoint[x:5 y:6]
      ]
    )"));
  }

  {
    QMouseEvent ev(
      QEvent::MouseMove,
      QPointF(11, 12),                 // local
      QPointF(13, 14),                 // window
      QPointF(15, 16),                 // screen
      Qt::NoButton,
      Qt::NoButton,
      Qt::NoModifier);

    EXPECT_EQ_GDV(toGDValue(ev), fromGDVN(R"(
      QMouseEvent[
        type: MouseMove
        spontaneous: false
        isAccepted: true
        modifiers: {}
        timestamp: 0
        button: NoButton
        buttons: {}
        pos: QPoint[x:11 y:12]
        windowPos: QPointF[x:13.0 y:14.0]
        globalPos: QPoint[x:15 y:16]
      ]
    )"));
  }
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_qt()
{
  test_QPoint();
  test_QPointF();
  test_QRect();
  test_QSize();
  test_QString();
  test_QEvent_Type();
  test_KeyboardModifier();
  test_MouseButton();
  test_KeyboardModifiers();
  test_MouseButtons();
  test_QMouseEvent();
}


// EOF
