// gdvalue-qrect.h
// Conversion between `GDValue` and `QRect`.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_GDVALUE_QRECT_H
#define SMQTUTIL_GDVALUE_QRECT_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue

class QRect;


gdv::GDValue toGDValue(QRect const &r);


#endif // SMQTUTIL_GDVALUE_QRECT_H
