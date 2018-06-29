// qtguiutil.cc
// code for qtguiutil.h

#include "qtguiutil.h"                 // this module

#include "qtutil.h"                    // toString for Key and Modifiers

#include <QKeyEvent>


string toString(QKeyEvent const &k)
{
  return stringc << toString(k.modifiers())
                 << "+" << toString((Qt::Key)(k.key()));
}


// EOF
