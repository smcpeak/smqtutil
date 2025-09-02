// gdvalue-qt.h
// Conversion between `GDValue` and Qt types.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_GDVALUE_QT_H
#define SMQTUTIL_GDVALUE_QT_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [m]
#include "smbase/gdvalue-parser-fwd.h" // gdv::GDVPTo [n]

class QPoint;
class QRect;
class QSize;
class QString;


gdv::GDValue toGDValue(QPoint const &p);
gdv::GDValue toGDValue(QRect const &r);
gdv::GDValue toGDValue(QString const &str);


gdv::GDValue toGDValue(QSize const &sz);

namespace gdv {
  template <>
  struct GDVPTo<QSize> {
    static QSize f(GDValueParser const &p);
  };
}

#endif // SMQTUTIL_GDVALUE_QT_H
