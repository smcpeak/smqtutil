// gdvalue-qt-test.cc
// Tests for `gdvalue-qt` module.

#include "gdvalue-qt.h"                // module under test

#include "smbase/gdvalue.h"            // gdv::toGDValue
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

#include <QPoint>
#include <QRect>
#include <QString>

using namespace gdv;


OPEN_ANONYMOUS_NAMESPACE


void test_QPoint()
{
  EXPECT_EQ(toGDValue(QPoint(4,5)).asString(), "QPoint[x:4 y:5]");
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


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_gdvalue_qt()
{
  test_QPoint();
  test_QRect();
  test_QString();
}


// EOF
