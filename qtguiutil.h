// qtguiutil.h
// Qt utilities that require Qt5Gui or Qt5Widgets as opposed to Qt5Core.

// I have split this off from 'qtutil.h' so I can have programs that
// only link with Qt5Core.  This gets included in libsmqtutil.a, but
// the linker will only pull it in to an executable if one of its
// symbols is used.

#ifndef QTGUIUTIL_H
#define QTGUIUTIL_H

#include "exc.h"                       // xBase
#include "str.h"                       // string

class QKeyEvent;
class QWidget;

// Render as a string for debugging.
string toString(QKeyEvent const &k);


// Display an unhandled exception error in a message box.  Limits
// itself to showing five dialog boxes.
void unhandledExceptionMsgbox(QWidget *parent, xBase const &x);


#endif // QTGUIUTIL_H
