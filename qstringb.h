// qstringb.h
// `qstringb` macro.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_QSTRINGB_H
#define SMQTUTIL_QSTRINGB_H

#include "smqtutil/qtutil.h"           // toQString(std::string)

#include "smbase/stringb.h"            // stringb


// The macro itself has no dependencies, but what it expands to has
// substantial dependencies, so I put it into its own module.
#define qstringb(stuff) toQString(stringb(stuff))


#endif // SMQTUTIL_QSTRINGB_H
