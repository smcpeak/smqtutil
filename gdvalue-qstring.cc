// gdvalue-qstring.cc
// Code for `gdvalue-qstring` module.

#include "gdvalue-qstring.h"           // this module

#include "smqtutil/qtutil.h"           // toString(QString)

#include "smbase/gdvalue.h"            // gdv::GDValue

#include <QString>

using namespace gdv;


gdv::GDValue toGDValue(QString const &str)
{
  return GDValue(toString(str));
}


// EOF
