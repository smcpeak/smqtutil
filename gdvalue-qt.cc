// gdvalue-qt.cc
// Code for `gdvalue-qt` module.

#include "gdvalue-qt.h"                // this module

#include "smbase/gdvalue-parser.h"     // gdv::GDValueParser
#include "smbase/gdvalue.h"            // gdv::GDValue

#include "smqtutil/qtutil.h"           // toString(QString)

#include <QPoint>
#include <QRect>

using namespace gdv;


gdv::GDValue toGDValue(QPoint const &p)
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "QPoint"_sym);
  m.mapSetValueAtSym("x", p.x());
  m.mapSetValueAtSym("y", p.y());
  return m;
}


gdv::GDValue toGDValue(QRect const &r)
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "QRect"_sym);
  m.mapSetValueAtSym("left", r.left());
  m.mapSetValueAtSym("top", r.top());
  m.mapSetValueAtSym("width", r.width());
  m.mapSetValueAtSym("height", r.height());
  return m;
}


gdv::GDValue toGDValue(QString const &str)
{
  return GDValue(toString(str));
}


gdv::GDValue toGDValue(QSize const &sz)
{
  GDValue t(GDVK_TAGGED_TUPLE, "QSize"_sym);
  t.tupleSet(GDVTuple{sz.width(), sz.height()});
  return t;
}


namespace gdv {
  /*static*/ QSize GDVPTo<QSize>::f(GDValueParser const &p)
  {
    p.checkIsTaggedTuple("QSize", 2);
    return QSize(
      p.tupleGetValueAt(0).smallIntegerGet(),
      p.tupleGetValueAt(1).smallIntegerGet());
  }
}


// EOF
