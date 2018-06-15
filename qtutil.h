// qtutil.h
// some miscellaneous utilities for Qt

#ifndef QTUTIL_H
#define QTUTIL_H

#include <QColor>             // QRgb
#include <qnamespace.h>       // MouseButtons, KeyboardModifiers, Key
#include <qstring.h>          // QString

#include "str.h"              // string

class QKeyEvent;
class QPoint;
class QRect;
class QSize;


// Render various values and objects as a string, mainly
// intended for debugging purposes.
string toString(Qt::MouseButtons buttons);
string toString(Qt::KeyboardModifiers kmods);
string toString(QKeyEvent const &k);
char const *toString(Qt::Key k);
string toString(QPoint p);
string toString(QRect r);
string toString(QSize s);
string qrgbToString(QRgb rgba);

// Convert 'QString' to 'string'.
string toString(QString const &s);

// Convert 'string' to 'QString'.
QString toQString(string const &s);
#define qstringb(stuff) toQString(stringb(stuff))


#endif // QTUTIL_H
