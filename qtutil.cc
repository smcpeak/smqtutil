// qtutil.cc
// code for qtutil.h

#include "qtutil.h"                    // this module

// smbase
#include "smbase/datablok.h"           // DataBlock
#include "smbase/exc.h"                // xassert, xformatsb
#include "smbase/overflow.h"           // safeToInt
#include "smbase/parsestring.h"        // ParseString
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/string-util.h"        // doubleQuote

// Qt
#include <QByteArray>
#include <QCoreApplication>            // + qInstallMessageHandler
#include <QLibraryInfo>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QSize>

// libc++
#include <iostream>                    // std::{ostream, cerr, endl}
#include <sstream>                     // std::ostringstream
#include <string>                      // std::string
#include <string_view>                 // std::string_view
#include <vector>                      // std::vector

// libc
#include <assert.h>                    // assert
#include <stdio.h>                     // sprintf


INIT_TRACE("qtutil");


// If `flags` contains `flag.value`, increment `ct`, add its name to
// `sb` and remove its value from `flags`.
template <class T>
static void handleFlag(
  int &ct,
  std::ostringstream &sb,
  QFlags<T> &flags,
  EnumeratorName<T> const &flag)
{
  if (flags & flag.m_value) {
    if (ct++ > 0) {
      sb << "+";
    }
    sb << flag.m_name;
    flags ^= flag.m_value;
  }
}


// Render 'flags' as a string by using 'definitions' to decode it.
template <class T>
static std::string flagsToString(
  QFlags<T> flags,
  EnumeratorName<T> const *definitions,
  int numDefinitions,
  char const *noFlagsName)
{
  std::ostringstream sb;

  int ct = 0;
  for (int i=0; i < numDefinitions; i++) {
    handleFlag(ct, sb, flags, definitions[i]);
  }

  if (flags) {
    if (ct > 0) {
      sb << " (plus unknown flags: " << (int)flags << ")";
    }
    else {
      sb << "(unknown flags: " << (int)flags << ")";
    }
  }

  if (ct == 0) {
    sb << noFlagsName;
  }

  return sb.str();
}


// Convert a string back to a flag, or throw XFormat.
template <class T>
static T stringToFlag(std::string const &str,
                      EnumeratorName<T> const *definitions,
                      int numDefinitions,
                      char const *typeName)
{
  for (int i=0; i < numDefinitions; i++) {
    if (str == definitions[i].m_name) {
      return definitions[i].m_value;
    }
  }
  xformatsb("invalid " << typeName << " name \"" << str << "\"");
  return definitions[0].m_value;   // silence warning
}


#define FLAG_DEFN(flag) { Qt::flag, #flag },

static EnumeratorName<Qt::MouseButton> const mouseButtonDefinitions[] = {
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

std::string toString(Qt::MouseButtons buttons)
{
  return flagsToString<Qt::MouseButton>(
    buttons,
    mouseButtonDefinitions,
    TABLESIZE(mouseButtonDefinitions),
    "NoButton");
}


#define MODIFIER_FLAG_DEFN(key) { Qt::key##Modifier, #key },

static EnumeratorName<Qt::KeyboardModifier> const keyboardModifierDefinitions[] = {
  FLAG_DEFN(NoModifier)
  MODIFIER_FLAG_DEFN(Shift)
  { Qt::ControlModifier, "Ctrl" },
  MODIFIER_FLAG_DEFN(Alt)
  MODIFIER_FLAG_DEFN(Meta)
  MODIFIER_FLAG_DEFN(Keypad)
  MODIFIER_FLAG_DEFN(GroupSwitch)
};


std::string toString(Qt::KeyboardModifiers kmods)
{
  return flagsToString<Qt::KeyboardModifier>(
    kmods,
    keyboardModifierDefinitions,
    TABLESIZE(keyboardModifierDefinitions),
    "NoModifier");
}



Qt::KeyboardModifier getKeyboardModifierFromString(std::string const &str)
{
  return stringToFlag<Qt::KeyboardModifier>(
    str,
    keyboardModifierDefinitions,
    TABLESIZE(keyboardModifierDefinitions),
    "KeyboardModifier");
}


std::string toString(QPoint p)
{
  return stringb('(' << p.x() << ',' << p.y() << ')');
}


std::string toString(QRect r)
{
  return stringb('[' << toString(r.topLeft()) << '+' <<
                 toString(r.size()) << ']');
}


std::string qrgbToString(QRgb rgba)
{
  char tmp[10];
  int n = sprintf(tmp, "#%08X", (unsigned int)rgba);
  assert(n < TABLESIZE(tmp));
  return std::string(tmp);
}


std::string toString(QSize s)
{
  return stringb('(' << s.width() << ',' << s.height() << ')');
}


QSize qSizeFromString(std::string const &str)
{
  ParseString ps(str);
  ps.parseByte('(');
  int w = ps.parseDecimalUInt();
  ps.parseByte(',');
  int h = ps.parseDecimalUInt();
  ps.parseByte(')');
  ps.parseEOS();

  return QSize(w, h);
}


QPoint toQPoint(QSize const &size)
{
  return QPoint(size.width(), size.height());
}

QSize toQSize(QPoint const &point)
{
  return QSize(point.x(), point.y());
}


bool isModifierKey(int key)
{
  switch (key) {
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
      return true;

    default:
      return false;
  }
}


static EnumeratorName<Qt::Key> const s_qtKeyNameTable[] = {
  #define HANDLE_KEY(key) { Qt::key, #key },
  #include "keys.incl"
  #undef HANDLE_KEY
};

EnumerationNames<Qt::Key> const g_qtKeyNames = {
  s_qtKeyNameTable,
  TABLESIZE(s_qtKeyNameTable)
};


std::string toString(QString const &s)
{
  QByteArray utf8(s.toUtf8());
  return std::string(utf8.constData(), utf8.length());
}


std::string doubleQuote(QString const &s)
{
  return doubleQuote(toString(s));
}


std::ostream& operator<< (std::ostream &os, QString const &str)
{
  return os << toString(str);
}


QString toQString(std::string const &s)
{
  return QString::fromUtf8(s.data(), safeToInt(s.size()));
}


QString toQString(std::string_view sv)
{
  return QString::fromUtf8(sv.data(), safeToInt(sv.size()));
}


QString toQString(char const *s)
{
  std::string_view sv(s);
  return toQString(sv);
}


std::vector<std::string> qStringListToStringVector(
  QStringList const &strList)
{
  std::vector<std::string> ret;

  for (QString const &s : strList) {
    ret.push_back(toString(s));
  }

  return ret;
}


std::string qObjectDesc(QObject *obj)
{
  if (obj) {
    return stringb(
      "{ptr=" << (void*)obj <<
      " name=" << doubleQuote(obj->objectName()) <<
      " class=" << obj->metaObject()->className() <<
      '}');
  }
  else {
    return "null";
  }
}


void setQObjectName(QObject *obj, char const *name)
{
  obj->setObjectName(name);
}


void disconnectSignalSender(QObject *sender)
{
  QObject::disconnect(sender, nullptr, nullptr, nullptr);
}


std::string qObjectPath(QObject const *obj)
{
  if (!obj) {
    return "null";
  }

  if (!obj->parent()) {
    // Root object.
    return toString(obj->objectName());
  }
  else {
    std::ostringstream sb;
    sb << qObjectPath(obj->parent()) << '.';
    if (obj->objectName().isEmpty()) {
      // I need a pointer to non-const to invoke 'indexOf'.  Of course
      // 'indexOf' will not change the object so this is fine.
      QObject *objnc = const_cast<QObject*>(obj);

      // Identify the child by number.  This is of course less reliable
      // than a name but is better than using an empty string.
      sb << '#' << obj->parent()->children().indexOf(objnc);
    }
    else {
      sb << obj->objectName();
    }
    return sb.str();
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


Qt::Key getKeyFromString(std::string const &str)
{
  // This is very inefficient.  I doubt it matters.
  #define HANDLE_KEY(key) \
    if (str == #key) { return Qt::key; }
  #include "keys.incl"
  #undef HANDLE_KEY

  xformatsb("unknown Key \"" << str << "\"");
  return Qt::Key_Escape;  // silence warning
}


void waitForQtEvent()
{
  // If no event is pending, block until one is.  Then process all
  // pending events.
  TRACE2("waitForQtEvent: start: calling processEvents");
  QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);

  // At least on Windows, `processEvents` begins by calling
  // `sendPostedEvents` (which does not report on whether it did
  // anything), and then it waits for an incoming IPC event.  But that
  // means the state change caused by dispatching intraprocess events
  // might not be seen, since we block for IPC regardless.  Therefore,
  // whenever we think we've gotten some IPC done, also drain the
  // intraprocess queue so the caller can see all of the effects of that
  // IPC before deciding whether to block again.
  //
  // This seems like a bug in Qt...
  //
  TRACE2("waitForQtEvent: middle: calling sendPostedEvents");
  QCoreApplication::sendPostedEvents();

  TRACE2("waitForQtEvent: end");
}


// Map a QtMsgType to a string for use when printing out a message that
// was sent with that type.
static char const *toString(QtMsgType mtype)
{
  switch (mtype) {
    case QtDebugMsg:         return "debug";
    case QtInfoMsg:          return "info";
    case QtWarningMsg:       return "warning";
    case QtCriticalMsg:      return "critical";
    case QtFatalMsg:         return "fatal";
    default:                 return "error";
  }
}


static void customMessageHandler(
  QtMsgType mtype,
  QMessageLogContext const &,
  QString const &message)
{
  if (message.indexOf("setGeometry: Unable to set geometry") >= 0) {
    // This message is generated anytime I use (e.g.) Alt+G to go to a
    // line, which uses 'QInputDialog::getText'.  It is caused by a bug
    // in Qt:
    //
    //   https://bugreports.qt.io/browse/QTBUG-73258
    //   https://stackoverflow.com/questions/54307407/why-am-i-getting-qwindowswindowsetgeometry-unable-to-set-geometry-warning-wit
    //
    // Apparently the only solution is to suppress the message.
    return;
  }

  // Print the message.
  std::cerr << toString(mtype) << ": " << message << std::endl;

  // Provide additional advice about the plugin error.
  if (message.indexOf("platform plugin") >= 0) {
    QString pluginsPath = QLibraryInfo::location(QLibraryInfo::PluginsPath);
    QString execPath = QCoreApplication::applicationDirPath();

    std::cerr <<
"\n"
"hint: When Qt complains about the \"platform plugin\", it means it is\n"
"looking for a file called \"platforms/q<platform>.{dll,so}\", where\n"
"<platform> is the name of that platform.\n"
"\n"
"By default, it looks in: " << pluginsPath << "\n"
"as well as relative to the program executable: " << execPath << "\n"
"\n"
"One way to fix this is to set QT_PLUGIN_PATH to point at a directory\n"
"containing the needed file.  On Windows, this might look like:\n"
"\n"
"  $ QT_PLUGIN_PATH=$(cygpath -m $HOME/opt/qt-5.9.9/plugins) " << execPath << "\n"
"\n"
"Another way might be to change PATH (on Windows) or LD_LIBRARY_PATH\n"
"(on Linux) to point at a different set of Qt DLLs, since the plugin\n"
"path is embedded in those DLLs.\n"
         << std::endl;
  }
}


void installSMQtUtilMessageHandler()
{
  qInstallMessageHandler(customMessageHandler);
}


// EOF
