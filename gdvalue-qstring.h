// gdvalue-qstring.h
// Conversion between `GDValue` and `QString`.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_GDVALUE_QSTRING_H
#define SMQTUTIL_GDVALUE_QSTRING_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue
#include "smbase/sm-macros.h"          // OPEN_NAMESPACE

class QString;


// TODO: These don't really belong in `gdv` but my
// `GDV_WRITE_MEMBER_SYM` macro demands it.
OPEN_NAMESPACE(gdv)


// Return a string GDValue.
gdv::GDValue toGDValue(QString const &str);


CLOSE_NAMESPACE(gdv)


#endif // SMQTUTIL_GDVALUE_QSTRING_H
