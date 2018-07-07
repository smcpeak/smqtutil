// qtguiutil.cc
// code for qtguiutil.h

#include "qtguiutil.h"                 // this module

#include "qtutil.h"                    // toString for Key and Modifiers

#include <QKeyEvent>
#include <QMessageBox>


string toString(QKeyEvent const &k)
{
  return stringc << toString(k.modifiers())
                 << "+" << toString((Qt::Key)(k.key()));
}


void unhandledExceptionMsgbox(QWidget *parent, xBase const &x)
{
  // Print to stderr as well.
  printUnhandled(x);

  static int count = 0;
  if (++count >= 5) {
    // Stop showing dialog boxes at a certain point.
    return;
  }

  QMessageBox::information(parent, "Oops",
    QString(stringc << "Unhandled exception: " << x.why() << "\n"
                    << "Save your work if you can!"));
}


// EOF
