// gdvalue-qrect.cc
// Code for `gdvalue-qrect` module.

#include "gdvalue-qrect.h"             // this module

#include "smbase/gdvalue.h"           // gdv::GDValue

#include <QRect>

using namespace gdv;


gdv::GDValue toGDValue(QRect const &r)
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "QRect"_sym);
  m.mapSetValueAtSym("left", r.left());
  m.mapSetValueAtSym("top", r.top());
  m.mapSetValueAtSym("width", r.width());
  m.mapSetValueAtSym("height", r.height());
  return m;
}


// EOF
