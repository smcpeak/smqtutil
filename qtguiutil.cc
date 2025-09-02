// qtguiutil.cc
// code for qtguiutil.h

#include "qtguiutil.h"                 // this module

// smqtutil
#include "smqtutil/gdvalue-qt.h"       // toGDValue({QRect,QPoint})
#include "smqtutil/qstringb.h"         // qstringb
#include "smqtutil/qtutil.h"           // toString for Key and Modifiers

// smbase
#include "smbase/gdvalue.h"            // gdv::toGDValue for TRACE1_GDVN_EXPRS
#include "smbase/exc.h"                // smbase::{xformat, XBase}
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/sm-windows.h"         // RECT, GetWindowRect
#include "smbase/string-util.h"        // doubleQuote
#include "smbase/stringb.h"            // stringb
#include "smbase/syserr.h"             // xsyserror

// Qt
#include <qtcoreversion.h>             // QTCORE_VERSION
#include <QApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QPainter>
#include <QShortcutEvent>
#include <QString>
#include <QStringList>
#include <QWidget>

// libc
#include <string.h>                    // memcmp

using namespace gdv;
using namespace smbase;


INIT_TRACE("qtguiutil");


string keysString(QKeyEvent const &k)
{
  // When the key is a modifier key, QKeyEvent::modifiers() flips the
  // corresponding bit!  Use QInputEvent::modifiers() instead to get
  // the data with which the object was originally constructed.
  Qt::KeyboardModifiers mods = k.QInputEvent::modifiers();

  if (mods == Qt::NoModifier) {
    return toString(Qt::Key(k.key()));
  }
  else {
    return stringb(toString(mods) << '+' << toString(Qt::Key(k.key())));
  }
}


static QKeyEvent *getKeyPressOrReleaseEventFromString(
  QEvent::Type eventType,
  string const &keys,
  QString const &text)
{
  try {
    QStringList keyList(toQString(keys).split('+'));
    if (keyList.isEmpty()) {
      xformat("no keys in string");
    }

    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    for (int i=0; i < keyList.count()-1; i++) {
      modifiers |= getKeyboardModifierFromString(toString(keyList.at(i)));
    }

    Qt::Key key = getKeyFromString(toString(keyList.last()));

    return new QKeyEvent(eventType, key, modifiers, text);
  }
  catch (XFormat &msg) {
    xformatsb("in key string \"" << keys << "\": " << msg.cond());
  }
}


QKeyEvent *getKeyPressEventFromString(string const &keys,
                                      QString const &text)
{
  return getKeyPressOrReleaseEventFromString(QEvent::KeyPress, keys, text);
}


QKeyEvent *getKeyReleaseEventFromString(string const &keys,
                                        QString const &text)
{
  return getKeyPressOrReleaseEventFromString(QEvent::KeyRelease, keys, text);
}


QKeySequence parseKeySequence(string const &keys)
{
  try {
    QKeySequence kseq(QKeySequence::fromString(toQString(keys)));
    if (kseq.count() < 1) {
      // This happens if 'keys' is empty.
      xformat("no keys");
    }

    // The documentation does not explain this, but 'fromString'
    // returns Qt::Key_unknown when it cannot parse the string.
    if (kseq[0] == Qt::Key_unknown) {
      xformat("unrecognized keys");
    }

    // Double-check that parsing succeeded, and also enforce a
    // canonical order to the modifiers, by requiring that we get the
    // original string by converting back.
    if (kseq.toString() != toQString(keys)) {
      xformat("not in canonical representation");
    }

    return kseq;
  }
  catch (XFormat &msg) {
    xformatsb("in key string " << doubleQuote(keys) << ": " << msg.cond());
  }
}


QShortcutEvent *getShortcutEventFromString(string const &keys)
{
  QKeySequence kseq(parseKeySequence(keys));

  // So far, the replay process appears to not be sensitive to the ID.
  // It would be easy to record it, of course, but I do not think it is
  // stable over time.
  //
  // Update: In some cases the proper ID is needed, such as when opening
  // a menu.  The problem is that the numbers themselves are not stable,
  // and there seems to be no way to programmatically get the ID from,
  // say, a path to the menu action.  (Qt stores the needed info but
  // does not expose it in the public API.)  So I'm stuck with passing 0
  // here when I can, and using some other approach (like directly
  // invoking menu actions) when I can't.
  return new QShortcutEvent(kseq, 0 /*id*/);
}


void unhandledExceptionMsgbox(QWidget *parent, XBase const &x)
{
  // Print to stderr as well.
  printUnhandled(x);

  static int count = 0;
  if (++count >= 5) {
    // Stop showing dialog boxes at a certain point.
    return;
  }

  QMessageBox::critical(parent, "Oops",
    qstringb("Unhandled exception: " << x.why() << "\n" <<
             "Save your work if you can!"));
}


void messageBox(QWidget *parent, QString title, QString message)
{
  QMessageBox box(parent);
  box.setObjectName(title);  // Give it a name for use in test scripts.
  box.setWindowTitle(title);
  box.setText(message);
  box.addButton(QMessageBox::Ok);
  box.exec();
}


void messageBox_details(QWidget *parent, QString title,
                        QString message, QString details)
{
  QMessageBox box(parent);
  box.setWindowTitle(title);
  box.setText(message);
  box.setDetailedText(details);
  box.addButton(QMessageBox::Ok);
  box.exec();
}


bool questionBoxYesCancel(QWidget *parent, QString title, QString question)
{
  QMessageBox box(parent);
  box.setWindowTitle(title);
  box.setText(question);
  box.addButton(QMessageBox::Yes);
  box.addButton(QMessageBox::Cancel);
  return (box.exec() == QMessageBox::Yes);
}


void centerWindowOnWindow(QWidget *windowToMove, QWidget *targetWindow)
{
  QPoint targetPt =
    targetWindow->pos() + toQPoint(targetWindow->size() / 2);
  windowToMove->move(targetPt - toQPoint(windowToMove->size() / 2));
}


void removeWindowContextHelpButton(QWidget *window)
{
#if QTCORE_VERSION >= 0x050900
  window->setWindowFlag(Qt::WindowContextHelpButtonHint, false /*on*/);
#endif
}


void showRaiseAndActivateWindow(QWidget *window)
{
  // See related discussion in:
  // https://stackoverflow.com/questions/7817334/qt-correct-way-to-show-display-raise-window

  // Visible.
  window->show();

  // Un-minimize.
  window->showNormal();

  // Bring it to the front.
  window->raise();

  // Give it focus.
  window->activateWindow();
}


QRect getTrueFrameGeometry(QWidget *window)
{
  xassertPrecondition(window != nullptr);
  xassertPrecondition(window->isWindow());

  if (PLATFORM_IS_WINDOWS) {
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    RECT r;
    if (GetWindowRect(hwnd, &r)) {
      // The rectangle returned has its `right` and `bottom` as the
      // pixel just *outside* the window, so subtracting them yields the
      // correct number of pixels for the width and height.
      return QRect(
        r.left,
        r.top,
        r.right - r.left,
        r.bottom - r.top);
    }
    else {
      xsyserror("GetWindowRect");
    }
  }
  else {
    // I don't know if the Qt functions work correctly on other
    // platforms.
  }

  // With the Windows theme I am using, this fails to account for the
  // thick (~7 pixel) window frames.  It also seems very confused
  // regarding the window height, omitting both the title bar height and
  // the bottom frame.
  return window->frameGeometry();
}


void setTrueFrameGeometry(
  QWidget *window, QRect const &desiredFrameRect)
{
  // Get the frame and interior geometries.
  QRect frameRect = getTrueFrameGeometry(window);
  QRect innerRect = window->geometry();

  // "inner - frame" is how to get to "inner" from "frame".  So add that
  // to the desired frame rect to get the desired inner rect.
  QRect desiredInnerRect(
    desiredFrameRect.topLeft() +
      (innerRect.topLeft() - frameRect.topLeft()),
    desiredFrameRect.bottomRight() +
      (innerRect.bottomRight() - frameRect.bottomRight()));

  window->setGeometry(desiredInnerRect);

  // Try to see if it worked.  This call only gets accurate info if the
  // window is currently visible, but the preceding calls appear to work
  // regardless.
  QRect actualFrameRect = getTrueFrameGeometry(window);

  TRACE1_GDVN_EXPRS("setTrueFrameGeometry",
    desiredFrameRect,
    frameRect,
    innerRect,
    desiredInnerRect,
    actualFrameRect);
}


void trueMoveWindow(QWidget *window, QPoint desiredTopLeft)
{
  xassertPrecondition(window != nullptr);
  xassertPrecondition(window->isWindow());

  // How much we needed to adjust last time.
  static QPoint savedCorrection(0,0);

  // Copy that into a local just in case this ends up being used from
  // multiple threads (in which case this is not sufficient, but better
  // than reading it multiple times).
  QPoint const prevCorrection = savedCorrection;

  // Try using the same correction.
  window->move(desiredTopLeft + prevCorrection);

  // Where did it end up?
  QRect const actual = getTrueFrameGeometry(window);

  // How far does it need to move to get to the right spot?
  QPoint const additionalCorrection = desiredTopLeft - actual.topLeft();

  if (!additionalCorrection.isNull()) {
    // Adjust the overall correction.
    QPoint const newCorrection = prevCorrection + additionalCorrection;

    // Apply the correction.
    window->move(desiredTopLeft + newCorrection);

    // I would like to confirm this worked, but for some reason calling
    // `getTrueFrameGeometry` a second time does not work, and just
    // returns the same thing as on the previous call, even when the
    // window *has* now moved to the correct position.

    TRACE1_GDVN_EXPRS("trueMoveWindow used one adjustment",
      desiredTopLeft,
      prevCorrection,
      actual,
      additionalCorrection,
      newCorrection);

    // Remember the updated correction value.
    savedCorrection = newCorrection;
  }
  else {
    TRACE1_GDVN_EXPRS("trueMoveWindow worked using saved adjustment",
      desiredTopLeft,
      prevCorrection,
      actual);
  }
}


// ------------------------- CursorSetRestore --------------------------
CursorSetRestore::CursorSetRestore(QWidget *w, QCursor const &newCursor)
  : m_widget(w),
    m_previousCursor(w->cursor())
{
  m_widget->setCursor(newCursor);
}

CursorSetRestore::~CursorSetRestore()
{
  m_widget->setCursor(m_previousCursor);
}


// --------------------- OverrideCursorSetRestore ----------------------
OverrideCursorSetRestore::OverrideCursorSetRestore(QCursor const &newCursor)
{
  QGuiApplication::setOverrideCursor(newCursor);
}


OverrideCursorSetRestore::~OverrideCursorSetRestore()
{
  QGuiApplication::restoreOverrideCursor();
}


// ------------------------ QPainterSaveRestore ------------------------
QPainterSaveRestore::QPainterSaveRestore(QPainter &painter)
  : m_painter(painter)
{
  m_painter.save();
}


QPainterSaveRestore::~QPainterSaveRestore()
{
  m_painter.restore();
}


// EOF
