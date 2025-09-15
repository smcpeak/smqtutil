// gdvalue-qt.h
// Conversion between `GDValue` and Qt types.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_GDVALUE_QT_H
#define SMQTUTIL_GDVALUE_QT_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [m]
#include "smbase/gdvalue-parser-fwd.h" // gdv::GDVPTo [n]

#include <QEvent>                      // QEvent::Type
#include <qnamespace.h>                // MouseButton[s], KeyboardModifier[s]

class QPoint;
class QPointF;
class QRect;
class QSize;
class QString;
class QMouseEvent;


gdv::GDValue toGDValue(QPoint const &p);
gdv::GDValue toGDValue(QPointF const &p);
gdv::GDValue toGDValue(QRect const &r);
gdv::GDValue toGDValue(QString const &str);


gdv::GDValue toGDValue(QSize const &sz);

namespace gdv {
  template <>
  struct GDVPTo<QSize> {
    static QSize f(GDValueParser const &p);
  };
}


// Return the given value as a symbol if recognized, and a tagged tuple
// carrying the numeric value if not.
gdv::GDValue toGDValue(QEvent::Type eventType);
gdv::GDValue toGDValue(Qt::KeyboardModifier mod);
gdv::GDValue toGDValue(Qt::MouseButton button);

// Return a set.
gdv::GDValue toGDValue(Qt::KeyboardModifiers mods);
gdv::GDValue toGDValue(Qt::MouseButtons buttons);


// All event details as an ordered map.
gdv::GDValue toGDValue(QMouseEvent const &mev);


#endif // SMQTUTIL_GDVALUE_QT_H
