// gdvalue-qt.h
// Conversion between `GDValue` and Qt types.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_GDVALUE_QT_H
#define SMQTUTIL_GDVALUE_QT_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue

class QPoint;
class QRect;
class QString;


gdv::GDValue toGDValue(QPoint const &p);
gdv::GDValue toGDValue(QRect const &r);
gdv::GDValue toGDValue(QString const &str);


#endif // SMQTUTIL_GDVALUE_QT_H
