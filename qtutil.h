// qtutil.h
// some miscellaneous utilities for Qt

#ifndef QTUTIL_H
#define QTUTIL_H

#include <QColor>             // QRgb
#include <qnamespace.h>       // MouseButtons, KeyboardModifiers, Key
#include <qstring.h>          // QString

#include "str.h"              // string

class QByteArray;
class QObject;
class QPoint;
class QRect;
class QSize;


// Render various values and objects as a string, mainly
// intended for debugging purposes.
string toString(Qt::MouseButtons buttons);
string toString(Qt::KeyboardModifiers kmods);
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

// Return a description of 'obj': either "null", or the pointer
// value and the object name.
string qObjectDesc(QObject *obj);

// Print the contents of 'ba' to stdout with 'label', then flush.
// The format is a hexdump with an ASCII column on the side.
void printQByteArray(QByteArray const &ba, char const *label);


#endif // QTUTIL_H
