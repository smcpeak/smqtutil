// gdvalue-qt.h
// Conversion between `GDValue` and Qt types.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_GDVALUE_QT_H
#define SMQTUTIL_GDVALUE_QT_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue [m]
#include "smbase/gdvalue-parser-fwd.h" // gdv::GDVPTo [n]

#include <QEvent>                      // QEvent::Type
#include <qnamespace.h>                // MouseButton[s], KeyboardModifier[s]

class QMouseEvent;
class QPoint;
class QPointF;
class QRect;
class QResizeEvent;
class QSize;
class QString;


gdv::GDValue toGDValue(QPoint const &p);
gdv::GDValue toGDValue(QPointF const &p);
gdv::GDValue toGDValue(QRect const &r);
gdv::GDValue toGDValue(QString const &str);


gdv::GDValue toGDValue(QSize const &sz);

// TODO: Move this into `gdvalue-parser.h`?
#define DECLARE_GDVPTO(Type)                 \
  namespace gdv {                            \
    template <>                              \
    struct GDVPTo<Type> {                    \
      static Type f(GDValueParser const &p); \
    };                                       \
  }

DECLARE_GDVPTO(QSize);
DECLARE_GDVPTO(QPoint);


// Return the given value as a symbol if recognized, and a tagged tuple
// carrying the numeric value if not.
gdv::GDValue toGDValue(QEvent::Type eventType);
gdv::GDValue toGDValue(Qt::KeyboardModifier mod);
gdv::GDValue toGDValue(Qt::MouseButton button);

// Return a set.
gdv::GDValue toGDValue(Qt::KeyboardModifiers mods);
gdv::GDValue toGDValue(Qt::MouseButtons buttons);

// Parse the above.
DECLARE_GDVPTO(QEvent::Type);
DECLARE_GDVPTO(Qt::KeyboardModifier);
DECLARE_GDVPTO(Qt::KeyboardModifiers);
DECLARE_GDVPTO(Qt::MouseButton);
DECLARE_GDVPTO(Qt::MouseButtons);

#undef DECLARE_GDVPTO


// All event details as an ordered map.
gdv::GDValue toGDValue(QMouseEvent const &ev);
gdv::GDValue toGDValue(QResizeEvent const &ev);


// Put the global `toGDValue` functions into the `Qt` namespace as well
// so they be found by ADL when working with `Qt::KeyboardModifier`,
// etc., when calling a function template that has "using
// gdv::toGDValue;" in it (such as `gdvnTestRoundtripEq`).
namespace Qt {
  using ::toGDValue;
}


#endif // SMQTUTIL_GDVALUE_QT_H
