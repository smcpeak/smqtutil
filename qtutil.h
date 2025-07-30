// qtutil.h
// some miscellaneous utilities for Qt.

#ifndef SMQTUTIL_QTUTIL_H
#define SMQTUTIL_QTUTIL_H

#include "smbase/std-string-fwd.h"               // std::string
#include "smbase/std-string-view-fwd.h"          // std::string_view

#include <QColor>                                // QRgb
#include <qnamespace.h>                          // MouseButtons, KeyboardModifiers, Key

#include <iosfwd>                                // std::ostream


class QByteArray;
class QObject;
class QPoint;
class QRect;
class QSize;
class QString;


// The numeric value and name of some enumerator.
template <class T>
struct EnumeratorName {
  // The numeric value.
  T m_value;

  // Textual name, as a pointer to a statically allocated string.
  char const *m_name;
};

template <class T>
struct EnumerationNames {
  // Pointer to a statically allocated array of names.
  EnumeratorName<T> const *m_names;

  // Number of elements in 'm_names'.
  int m_size;
};


// Render various values and objects as a string.
std::string toString(Qt::MouseButtons buttons);
std::string toString(Qt::KeyboardModifiers kmods);
char const *toString(Qt::Key k);
std::string toString(QPoint p);
std::string toString(QRect r);
std::string qrgbToString(QRgb rgba);


// Convert QSize to "($width,$height)".
std::string toString(QSize s);

// Convert "($width,$height)" to QSize or throw XFormat.
QSize qSizeFromString(std::string const &str);


// Convert between QSize and QPoint.
QPoint toQPoint(QSize const &size);
QSize toQSize(QPoint const &point);


// Convert a keyboard modifier name back to its number, or throw XFormat.
Qt::KeyboardModifier getKeyboardModifierFromString(std::string const &str);


// Convert a key to its number, or throw XFormat.
Qt::Key getKeyFromString(std::string const &str);

// True if 'key' is (exactly) Qt::Key_Shift, Control, Meta, Alt, or AltGr.
bool isModifierKey(int key);

// Table of key names.
extern EnumerationNames<Qt::Key> const g_qtKeyNames;


// Convert 'QString' to 'std::string'.
std::string toString(QString const &s);

// Equivalent to 'doubleQuote(toString(s))'.
std::string doubleQuote(QString const &s);


// Allow inserting QString into std::ostream.
//
// For a while I have been avoiding this on safety grounds, instead
// explicitly calling toString, but that is annoying and the safety
// benefit seems minimal, particularly as my eventual intent is for
// 8-bit characters to be UTF-8, and hence this does not lose
// information.
std::ostream& operator<< (std::ostream &os, QString const &str);

// Convert 'string' to 'QString'.
QString toQString(std::string const &s);

// Also convert `string_view` and `char*`.
QString toQString(std::string_view sv);
QString toQString(char const *s);


// Return a description of 'obj': either "null", or the pointer
// value, object name, and class name.
std::string qObjectDesc(QObject *obj);

// Set the name.
void setQObjectName(QObject *obj, char const *name);

// Set the name based on a variable (typically field) name.
#define SET_QOBJECT_NAME(object) setQObjectName(object, #object)


// Disconnect all signals where 'sender' is the sender.
//
// This is generally a good idea to do in a destructor, upon 'this' and
// all owned QObjects, since the activity of destruction can otherwise
// result in signals being sent to QObjects that have been at least
// partially destroyed.
void disconnectSignalSender(QObject *sender);


// Return a dotted sequence of object names from a root object down to
// this object, like "foo.bar.baz", or "null" if obj is NULL.  Where an
// object name is empty, the path simply has the empty string in that
// location, like "foo..bar".
std::string qObjectPath(QObject const *obj);


// Print the contents of 'ba' to stdout with 'label', then flush.
// The format is a hexdump with an ASCII column on the side.
void printQByteArray(QByteArray const &ba, char const *label);


// Block until an event happens (e.g., IPC, or timer expiring), then
// process that event and return.  If at least one event is already
// pending, process the pending events and return.
//
// The idea is you can write synchronous interactions on top of
// asynchronous interfaces like:
//
//   while (!someCondition()) {
//     waitForQtEvent();
//   }
//
void waitForQtEvent();


#endif // SMQTUTIL_QTUTIL_H
