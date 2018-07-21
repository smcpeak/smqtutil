// qtutil.h
// some miscellaneous utilities for Qt

#ifndef QTUTIL_H
#define QTUTIL_H

#include <QColor>             // QRgb
#include <qnamespace.h>       // MouseButtons, KeyboardModifiers, Key
#include <qstring.h>          // QString

#include "sm-iostream.h"      // ostream
#include "str.h"              // string, stringBuilder

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

// Convert a keyboard modifier name back to its number, or throw xFormat.
Qt::KeyboardModifier getKeyboardModifierFromString(string const &str);

// Convert a key to its number, or throw xFormat.
Qt::Key getKeyFromString(string const &str);


// Convert 'QString' to 'string'.
string toString(QString const &s);


// Equivalent to 'quoted(toString(s))'.
string quoted(QString const &s);


// Allow inserting QString into stringBuilder and ostream.
//
// For a while I have been avoiding this on safety grounds, instead
// explicitly calling toString, but that is annoying and the safety
// benefit seems minimal, particularly as my eventual intent is for
// 8-bit characters to be UTF-8, and hence this does not lose
// information.
stringBuilder& operator<< (stringBuilder& sb, QString const &str);
ostream& operator<< (ostream &os, QString const &str);

// Convert 'string' to 'QString'.
QString toQString(string const &s);
#define qstringb(stuff) toQString(stringb(stuff))


// Return a description of 'obj': either "null", or the pointer
// value and the object name.
string qObjectDesc(QObject *obj);

// Set the name.
void setQObjectName(QObject *obj, char const *name);

// Set the name based on a variable (typically field) name.
#define SET_QOBJECT_NAME(object) setQObjectName(object, #object)


// Return a dotted sequence of object names from a root object down to
// this object, like "foo.bar.baz", or "null" if obj is NULL.  Where an
// object name is empty, the path simply has the empty string in that
// location, like "foo..bar".
string qObjectPath(QObject const *obj);


// Print the contents of 'ba' to stdout with 'label', then flush.
// The format is a hexdump with an ASCII column on the side.
void printQByteArray(QByteArray const &ba, char const *label);


#endif // QTUTIL_H
