// gdvalue-qrect.h
// Conversion between `GDValue` and `QRect` and `QPoint`.

// See license.txt for copyright and terms of use.

// TODO: Consolidate `gdvalue-*` here.  And write tests.

#ifndef SMQTUTIL_GDVALUE_QRECT_H
#define SMQTUTIL_GDVALUE_QRECT_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue

class QPoint;
class QRect;


gdv::GDValue toGDValue(QPoint const &p);
gdv::GDValue toGDValue(QRect const &r);


#endif // SMQTUTIL_GDVALUE_QRECT_H
