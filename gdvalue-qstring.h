// gdvalue-qstring.h
// Conversion between `GDValue` and `QString`.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_GDVALUE_QSTRING_H
#define SMQTUTIL_GDVALUE_QSTRING_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue

class QString;


// Return a string GDValue.
gdv::GDValue toGDValue(QString const &str);


#endif // SMQTUTIL_GDVALUE_QSTRING_H
