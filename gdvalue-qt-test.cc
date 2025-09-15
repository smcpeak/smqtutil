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


void test_QEvent_Type()
{
  EXPECT_EQ_GDV(QEvent::MouseButtonRelease,
    fromGDVN("MouseButtonRelease"));
  EXPECT_EQ_GDV(QEvent::User,
    fromGDVN("User(1000)"));
}


void test_KeyboardModifier()
{
  EXPECT_EQ_GDV(Qt::AltModifier,
    fromGDVN("Alt"));
  EXPECT_EQ_GDV(Qt::KeyboardModifier(456),
    fromGDVN("UnknownKeyboardModifier(456)"));
}


void test_MouseButton()
{
  EXPECT_EQ_GDV(Qt::MiddleButton,
    fromGDVN("MiddleButton"));
  EXPECT_EQ_GDV(Qt::MouseButton(123),
    fromGDVN("UnknownMouseButton(123)"));
}


void test_KeyboardModifiers()
{
  EXPECT_EQ_GDV(Qt::KeyboardModifiers(Qt::NoModifier),
    fromGDVN("{}"));

  EXPECT_EQ_GDV(Qt::KeyboardModifiers(Qt::AltModifier),
    fromGDVN("{Alt}"));

  EXPECT_EQ_GDV(
    Qt::KeyboardModifiers(Qt::AltModifier | Qt::ShiftModifier),
    fromGDVN("{Alt Shift}"));

  EXPECT_EQ_GDV(
    Qt::KeyboardModifiers(Qt::ShiftModifier | Qt::KeyboardModifier(2)),
    fromGDVN("{Shift UnknownKeyboardModifier(2)}"));
}


void test_MouseButtons()
{
  EXPECT_EQ_GDV(Qt::MouseButtons(Qt::NoButton),
    fromGDVN("{}"));

  EXPECT_EQ_GDV(Qt::MouseButtons(Qt::LeftButton),
    fromGDVN("{LeftButton}"));

  EXPECT_EQ_GDV(
    Qt::MouseButtons(Qt::LeftButton | Qt::RightButton),
    fromGDVN("{LeftButton RightButton}"));

  // 0x1000 is actually `ExtraButton10`, but my table does not include
  // it, so it exercises the "unknown" case.
  EXPECT_EQ_GDV(
    Qt::MouseButtons(Qt::MiddleButton | Qt::MouseButton(0x1000)),
    fromGDVN("{MiddleButton UnknownMouseButton(4096)}"));
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
