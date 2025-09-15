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


#define QT_FLAG_DEFN(flag) { Qt::flag, #flag },

static EnumeratorName<Qt::MouseButton> const mouseButtonDefinitions[] = {
  // This list comes from qt5/qtbase/src/corelib/kernel/qnamespace.h.
  QT_FLAG_DEFN(NoButton)
  QT_FLAG_DEFN(LeftButton)
  QT_FLAG_DEFN(RightButton)
  QT_FLAG_DEFN(MiddleButton)
  QT_FLAG_DEFN(BackButton)
  QT_FLAG_DEFN(ForwardButton)
  QT_FLAG_DEFN(TaskButton)
  QT_FLAG_DEFN(ExtraButton4)
  QT_FLAG_DEFN(ExtraButton5)
  // ExtraButtons up to 24 are defined, but I'll stop here.
};

static EnumerationNames<Qt::MouseButton> const mouseButtonNames = {
  mouseButtonDefinitions,
  TABLESIZE(mouseButtonDefinitions)
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
  QT_FLAG_DEFN(NoModifier)
  MODIFIER_FLAG_DEFN(Shift)
  { Qt::ControlModifier, "Ctrl" },
  MODIFIER_FLAG_DEFN(Alt)
  MODIFIER_FLAG_DEFN(Meta)
  MODIFIER_FLAG_DEFN(Keypad)
  MODIFIER_FLAG_DEFN(GroupSwitch)
};

#undef MODIFIER_FLAG_DEFN
#undef QT_FLAG_DEFN


static EnumerationNames<Qt::KeyboardModifier> const keyboardModifierNames = {
  keyboardModifierDefinitions,
  TABLESIZE(keyboardModifierDefinitions)
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


static EnumeratorName<QEvent::Type> const eventTypeDefinitions[] = {
  #define EVENT_TYPE_DEFN(Enumerator) \
    { QEvent::Enumerator, #Enumerator },

  // This list comes from qt5/qtbase/src/corelib/kernel/qcoreevent.h.
  EVENT_TYPE_DEFN(None)
  EVENT_TYPE_DEFN(Timer)
  EVENT_TYPE_DEFN(MouseButtonPress)
  EVENT_TYPE_DEFN(MouseButtonRelease)
  EVENT_TYPE_DEFN(MouseButtonDblClick)
  EVENT_TYPE_DEFN(MouseMove)
  EVENT_TYPE_DEFN(KeyPress)
  EVENT_TYPE_DEFN(KeyRelease)
  EVENT_TYPE_DEFN(FocusIn)
  EVENT_TYPE_DEFN(FocusOut)
  EVENT_TYPE_DEFN(FocusAboutToChange)
  EVENT_TYPE_DEFN(Enter)
  EVENT_TYPE_DEFN(Leave)
  EVENT_TYPE_DEFN(Paint)
  EVENT_TYPE_DEFN(Move)
  EVENT_TYPE_DEFN(Resize)
  EVENT_TYPE_DEFN(Create)
  EVENT_TYPE_DEFN(Destroy)
  EVENT_TYPE_DEFN(Show)
  EVENT_TYPE_DEFN(Hide)
  EVENT_TYPE_DEFN(Close)
  EVENT_TYPE_DEFN(Quit)
  EVENT_TYPE_DEFN(ParentChange)
  EVENT_TYPE_DEFN(ParentAboutToChange)
  EVENT_TYPE_DEFN(ThreadChange)
  EVENT_TYPE_DEFN(WindowActivate)
  EVENT_TYPE_DEFN(WindowDeactivate)
  EVENT_TYPE_DEFN(ShowToParent)
  EVENT_TYPE_DEFN(HideToParent)
  EVENT_TYPE_DEFN(Wheel)
  EVENT_TYPE_DEFN(WindowTitleChange)
  EVENT_TYPE_DEFN(WindowIconChange)
  EVENT_TYPE_DEFN(ApplicationWindowIconChange)
  EVENT_TYPE_DEFN(ApplicationFontChange)
  EVENT_TYPE_DEFN(ApplicationLayoutDirectionChange)
  EVENT_TYPE_DEFN(ApplicationPaletteChange)
  EVENT_TYPE_DEFN(PaletteChange)
  EVENT_TYPE_DEFN(Clipboard)
  EVENT_TYPE_DEFN(Speech)
  EVENT_TYPE_DEFN(MetaCall)
  EVENT_TYPE_DEFN(SockAct)
  EVENT_TYPE_DEFN(WinEventAct)
  EVENT_TYPE_DEFN(DeferredDelete)
  EVENT_TYPE_DEFN(DragEnter)
  EVENT_TYPE_DEFN(DragMove)
  EVENT_TYPE_DEFN(DragLeave)
  EVENT_TYPE_DEFN(Drop)
  EVENT_TYPE_DEFN(DragResponse)
  EVENT_TYPE_DEFN(ChildAdded)
  EVENT_TYPE_DEFN(ChildPolished)
  EVENT_TYPE_DEFN(ChildRemoved)
  EVENT_TYPE_DEFN(ShowWindowRequest)
  EVENT_TYPE_DEFN(PolishRequest)
  EVENT_TYPE_DEFN(Polish)
  EVENT_TYPE_DEFN(LayoutRequest)
  EVENT_TYPE_DEFN(UpdateRequest)
  EVENT_TYPE_DEFN(UpdateLater)

  EVENT_TYPE_DEFN(EmbeddingControl)
  EVENT_TYPE_DEFN(ActivateControl)
  EVENT_TYPE_DEFN(DeactivateControl)
  EVENT_TYPE_DEFN(ContextMenu)
  EVENT_TYPE_DEFN(InputMethod)
  EVENT_TYPE_DEFN(TabletMove)
  EVENT_TYPE_DEFN(LocaleChange)
  EVENT_TYPE_DEFN(LanguageChange)
  EVENT_TYPE_DEFN(LayoutDirectionChange)
  EVENT_TYPE_DEFN(Style)
  EVENT_TYPE_DEFN(TabletPress)
  EVENT_TYPE_DEFN(TabletRelease)
  EVENT_TYPE_DEFN(OkRequest)
  EVENT_TYPE_DEFN(HelpRequest)

  EVENT_TYPE_DEFN(IconDrag)

  EVENT_TYPE_DEFN(FontChange)
  EVENT_TYPE_DEFN(EnabledChange)
  EVENT_TYPE_DEFN(ActivationChange)
  EVENT_TYPE_DEFN(StyleChange)
  EVENT_TYPE_DEFN(IconTextChange)
  EVENT_TYPE_DEFN(ModifiedChange)
  EVENT_TYPE_DEFN(MouseTrackingChange)

  EVENT_TYPE_DEFN(WindowBlocked)
  EVENT_TYPE_DEFN(WindowUnblocked)
  EVENT_TYPE_DEFN(WindowStateChange)

  EVENT_TYPE_DEFN(ReadOnlyChange)

  EVENT_TYPE_DEFN(ToolTip)
  EVENT_TYPE_DEFN(WhatsThis)
  EVENT_TYPE_DEFN(StatusTip)

  EVENT_TYPE_DEFN(ActionChanged)
  EVENT_TYPE_DEFN(ActionAdded)
  EVENT_TYPE_DEFN(ActionRemoved)

  EVENT_TYPE_DEFN(FileOpen)

  EVENT_TYPE_DEFN(Shortcut)
  EVENT_TYPE_DEFN(ShortcutOverride)

  EVENT_TYPE_DEFN(WhatsThisClicked)

  EVENT_TYPE_DEFN(ToolBarChange)

  EVENT_TYPE_DEFN(ApplicationActivate)
  EVENT_TYPE_DEFN(ApplicationDeactivate)

  EVENT_TYPE_DEFN(QueryWhatsThis)
  EVENT_TYPE_DEFN(EnterWhatsThisMode)
  EVENT_TYPE_DEFN(LeaveWhatsThisMode)

  EVENT_TYPE_DEFN(ZOrderChange)

  EVENT_TYPE_DEFN(HoverEnter)
  EVENT_TYPE_DEFN(HoverLeave)
  EVENT_TYPE_DEFN(HoverMove)

#ifdef QT_KEYPAD_NAVIGATION
  EVENT_TYPE_DEFN(EnterEditFocus)
  EVENT_TYPE_DEFN(LeaveEditFocus)
#endif
  EVENT_TYPE_DEFN(AcceptDropsChange)

  EVENT_TYPE_DEFN(ZeroTimerEvent)

  EVENT_TYPE_DEFN(GraphicsSceneMouseMove)
  EVENT_TYPE_DEFN(GraphicsSceneMousePress)
  EVENT_TYPE_DEFN(GraphicsSceneMouseRelease)
  EVENT_TYPE_DEFN(GraphicsSceneMouseDoubleClick)
  EVENT_TYPE_DEFN(GraphicsSceneContextMenu)
  EVENT_TYPE_DEFN(GraphicsSceneHoverEnter)
  EVENT_TYPE_DEFN(GraphicsSceneHoverMove)
  EVENT_TYPE_DEFN(GraphicsSceneHoverLeave)
  EVENT_TYPE_DEFN(GraphicsSceneHelp)
  EVENT_TYPE_DEFN(GraphicsSceneDragEnter)
  EVENT_TYPE_DEFN(GraphicsSceneDragMove)
  EVENT_TYPE_DEFN(GraphicsSceneDragLeave)
  EVENT_TYPE_DEFN(GraphicsSceneDrop)
  EVENT_TYPE_DEFN(GraphicsSceneWheel)

  EVENT_TYPE_DEFN(KeyboardLayoutChange)

  EVENT_TYPE_DEFN(DynamicPropertyChange)

  EVENT_TYPE_DEFN(TabletEnterProximity)
  EVENT_TYPE_DEFN(TabletLeaveProximity)

  EVENT_TYPE_DEFN(NonClientAreaMouseMove)
  EVENT_TYPE_DEFN(NonClientAreaMouseButtonPress)
  EVENT_TYPE_DEFN(NonClientAreaMouseButtonRelease)
  EVENT_TYPE_DEFN(NonClientAreaMouseButtonDblClick)

  EVENT_TYPE_DEFN(MacSizeChange)

  EVENT_TYPE_DEFN(ContentsRectChange)

  EVENT_TYPE_DEFN(MacGLWindowChange)

  EVENT_TYPE_DEFN(FutureCallOut)

  EVENT_TYPE_DEFN(GraphicsSceneResize)
  EVENT_TYPE_DEFN(GraphicsSceneMove)

  EVENT_TYPE_DEFN(CursorChange)
  EVENT_TYPE_DEFN(ToolTipChange)

  EVENT_TYPE_DEFN(NetworkReplyUpdated)

  EVENT_TYPE_DEFN(GrabMouse)
  EVENT_TYPE_DEFN(UngrabMouse)
  EVENT_TYPE_DEFN(GrabKeyboard)
  EVENT_TYPE_DEFN(UngrabKeyboard)
  EVENT_TYPE_DEFN(MacGLClearDrawable)

  EVENT_TYPE_DEFN(StateMachineSignal)
  EVENT_TYPE_DEFN(StateMachineWrapped)

  EVENT_TYPE_DEFN(TouchBegin)
  EVENT_TYPE_DEFN(TouchUpdate)
  EVENT_TYPE_DEFN(TouchEnd)

#ifndef QT_NO_GESTURES
  EVENT_TYPE_DEFN(NativeGesture)
#endif
  EVENT_TYPE_DEFN(RequestSoftwareInputPanel)
  EVENT_TYPE_DEFN(CloseSoftwareInputPanel)

  EVENT_TYPE_DEFN(WinIdChange)
#ifndef QT_NO_GESTURES
  EVENT_TYPE_DEFN(Gesture)
  EVENT_TYPE_DEFN(GestureOverride)
#endif
  EVENT_TYPE_DEFN(ScrollPrepare)
  EVENT_TYPE_DEFN(Scroll)

  EVENT_TYPE_DEFN(Expose)

  EVENT_TYPE_DEFN(InputMethodQuery)
  EVENT_TYPE_DEFN(OrientationChange)

  EVENT_TYPE_DEFN(TouchCancel)

  EVENT_TYPE_DEFN(ThemeChange)

  EVENT_TYPE_DEFN(SockClose)

  EVENT_TYPE_DEFN(PlatformPanel)

  EVENT_TYPE_DEFN(StyleAnimationUpdate)
  EVENT_TYPE_DEFN(ApplicationStateChange)

  EVENT_TYPE_DEFN(WindowChangeInternal)
  EVENT_TYPE_DEFN(ScreenChangeInternal)

  EVENT_TYPE_DEFN(PlatformSurface)

  EVENT_TYPE_DEFN(Pointer)

  EVENT_TYPE_DEFN(TabletTrackingChange)

  #undef EVENT_TYPE_DEFN
};


static EnumerationNames<QEvent::Type> const eventTypeNames = {
  eventTypeDefinitions,
  TABLESIZE(eventTypeDefinitions)
};


template <typename T>
char const * NULLABLE lookupEnumeratorName(
  T value,
  EnumerationNames<T> const &names)
{
  for (int i=0; i < names.m_size; ++i) {
    if (names.m_names[i].m_value == value) {
      return names.m_names[i].m_name;
    }
  }
  return nullptr;
}


char const * NULLABLE toStringOpt(QEvent::Type value)
{
  return lookupEnumeratorName(value, eventTypeNames);
}


char const * NULLABLE toStringOpt(Qt::KeyboardModifier value)
{
  return lookupEnumeratorName(value, keyboardModifierNames);
}


char const * NULLABLE toStringOpt(Qt::MouseButton value)
{
  return lookupEnumeratorName(value, mouseButtonNames);
}


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


void waitForQtEvent(bool processInputEvents)
{
  QEventLoop::ProcessEventsFlags flags = QEventLoop::WaitForMoreEvents;
  if (!processInputEvents) {
    flags |= QEventLoop::ExcludeUserInputEvents;
  }

  // If no event is pending, block until one is.  Then process all
  // pending events.
  TRACE2("waitForQtEvent(input=" << processInputEvents <<
         "): start: calling processEvents");
  QCoreApplication::processEvents(flags);

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
  TRACE2("waitForQtEvent(input=" << processInputEvents <<
         "): middle: calling sendPostedEvents");
  QCoreApplication::sendPostedEvents();

  TRACE2("waitForQtEvent(input=" << processInputEvents << "): end");
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

  /* Similar to the preceding case, this is a useless (to me) warning.
     This is a relatively informative discussion:

       https://github.com/flatpak/flatpak/issues/3397

     Among other things, it links to the change that introduced the
     check in Qt 5.13:

       https://codereview.qt-project.org/c/qt/qtbase/+/256521

     As I am using WSL, it seems likely the reason the permissions are
     wrong is that WSL has a bug (that Microsoft deems unimportant):

       https://github.com/microsoft/WSL/issues/10896

     In any case, my programs are not using /run/user, so I'm simply
     going to swallow the warning.
  */
  if (message.indexOf("QStandardPaths: wrong permissions on runtime directory") >= 0) {
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
