// qtutil.cc
// code for qtutil.h

#include "qtutil.h"                    // this module

// smase
#include "datablok.h"                  // DataBlock
#include "exc.h"                       // xassert

// Qt
#include <QByteArray>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QSize>

// libc
#include <stdio.h>                     // sprintf


// The numeric value and name of some enumerator flag.
template <class T>
struct FlagDefinition {
  T value;
  char const *name;
};


// If 'flags' contains 'flag.value', add its name to 'sb' and remove
// its value from 'flags'.
template <class T>
static void handleFlag(stringBuilder &sb, QFlags<T> &flags,
                       FlagDefinition<T> const &flag)
{
  if (flags & flag.value) {
    if (sb.length() > 0) {
      sb << "+";
    }
    sb << flag.name;
    flags ^= flag.value;
  }
}


// Render 'flags' as a string by using 'definitions' to decode it.
template <class T>
static string flagsToString(QFlags<T> flags,
                            FlagDefinition<T> const *definitions,
                            int numDefinitions,
                            char const *noFlagsName)
{
  stringBuilder sb;

  for (int i=0; i < numDefinitions; i++) {
    handleFlag(sb, flags, definitions[i]);
  }

  if (flags) {
    if (sb.length() > 0) {
      sb << " (plus unknown flags: " << (int)flags << ")";
    }
    else {
      sb << "(unknown flags: " << (int)flags << ")";
    }
  }

  if (sb.length() == 0) {
    sb << noFlagsName;
  }

  return sb;
}


// Convert a string back to a flag, or throw xFormat.
template <class T>
static T stringToFlag(string const &str,
                      FlagDefinition<T> const *definitions,
                      int numDefinitions,
                      char const *typeName)
{
  for (int i=0; i < numDefinitions; i++) {
    if (str == definitions[i].name) {
      return definitions[i].value;
    }
  }
  xformat(stringb("invalid " << typeName << " name \"" << str << "\""));
  return definitions[0].value;   // silence warning
}


#define FLAG_DEFN(flag) { Qt::flag, #flag },

static FlagDefinition<Qt::MouseButton> const mouseButtonDefinitions[] = {
  FLAG_DEFN(LeftButton)
  FLAG_DEFN(RightButton)
  FLAG_DEFN(MiddleButton)
  FLAG_DEFN(BackButton)
  FLAG_DEFN(ForwardButton)
  FLAG_DEFN(TaskButton)
  FLAG_DEFN(ExtraButton4)
  FLAG_DEFN(ExtraButton5)
  // ExtraButtons up to 24 are defined, but I'll stop here.
};

string toString(Qt::MouseButtons buttons)
{
  return flagsToString<Qt::MouseButton>(
    buttons,
    mouseButtonDefinitions,
    TABLESIZE(mouseButtonDefinitions),
    "NoButton");
}


static FlagDefinition<Qt::KeyboardModifier> const keyboardModifierDefinitions[] = {
  FLAG_DEFN(NoModifier)
  FLAG_DEFN(ShiftModifier)
  FLAG_DEFN(ControlModifier)
  FLAG_DEFN(AltModifier)
  FLAG_DEFN(MetaModifier)
  FLAG_DEFN(KeypadModifier)
  FLAG_DEFN(GroupSwitchModifier)
};


string toString(Qt::KeyboardModifiers kmods)
{
  return flagsToString<Qt::KeyboardModifier>(
    kmods,
    keyboardModifierDefinitions,
    TABLESIZE(keyboardModifierDefinitions),
    "NoModifier");
}



Qt::KeyboardModifier getKeyboardModifierFromString(string const &str)
{
  return stringToFlag<Qt::KeyboardModifier>(
    str,
    keyboardModifierDefinitions,
    TABLESIZE(keyboardModifierDefinitions),
    "KeyboardModifier");
}


string toString(QPoint p)
{
  return stringb('(' << p.x() << ',' << p.y() << ')');
}


string toString(QRect r)
{
  return stringb('[' << toString(r.topLeft()) << '+' <<
                 toString(r.size()) << ']');
}


string toString(QSize s)
{
  return stringb('(' << s.width() << ',' << s.height() << ')');
}


string qrgbToString(QRgb rgba)
{
  char tmp[10];
  int n = sprintf(tmp, "#%08X", (unsigned int)rgba);
  xassert(n < TABLESIZE(tmp));
  return string(tmp);
}


string toString(QString const &s)
{
  return string(s.toUtf8().constData());
}


stringBuilder& operator<< (stringBuilder& sb, QString const &str)
{
  return sb << str.toUtf8().constData();
}


ostream& operator<< (ostream &os, QString const &str)
{
  return os << str.toUtf8().constData();
}


QString toQString(string const &s)
{
  return QString(s.c_str());
}


string qObjectDesc(QObject *obj)
{
  if (obj) {
    return stringb((void*)obj << " (" <<
                   toString(obj->objectName()) << ')');
  }
  else {
    return "null";
  }
}


void printQByteArray(QByteArray const &ba, char const *label)
{
  // This is an inefficient but convenient implementation.
  DataBlock db(ba.constData(), ba.size());
  db.print(label);
}


char const *toString(Qt::Key k)
{
  #define HANDLE_KEY(key) \
    case Qt::key: return #key;

  switch (k) {
    #include "keys.incl"
    default: return "(unknown)";
  }

  #undef HANDLE_KEY
}


Qt::Key getKeyFromString(string const &str)
{
  // This is very inefficient.  I doubt it matters.
  #define HANDLE_KEY(key) \
    if (str == #key) { return Qt::key; }
  #include "keys.incl"
  #undef HANDLE_KEY

  xformatsb("unknown Key \"" << str << "\"");
  return Qt::Key_Escape;  // silence warning
}


// EOF
