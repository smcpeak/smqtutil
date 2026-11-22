// qtguiutil.h
// Qt utilities that require Qt5Gui or Qt5Widgets as opposed to Qt5Core.

// I have split this off from 'qtutil.h' so I can have programs that
// only link with Qt5Core.  This gets included in libsmqtutil.a, but
// the linker will only pull it in to an executable if one of its
// symbols is used.

#ifndef QTGUIUTIL_H
#define QTGUIUTIL_H

#include "smbase/exc.h"                // smbase::XBase
#include "smbase/sm-macros.h"          // NO_OBJECT_COPIES
#include "smbase/str.h"                // string

#include <QCursor>

class QKeyEvent;
class QKeySequence;
class QPainter;
class QPoint;
class QRect;
class QShortcutEvent;
class QString;
class QWidget;


// Render as a string for debugging or event record/replay.  This
// string just captures the key(s), not the text.
string keysString(QKeyEvent const &k);

// Convert the string back to a QKeyEvent for event replay.  The
// returned pointer is an owner pointer.  Throws XFormat if the string
// cannot be parsed as a KeyPress event.
QKeyEvent *getKeyPressEventFromString(string const &keys,
                                      QString const &text);

// Same, but as a release event.
QKeyEvent *getKeyReleaseEventFromString(string const &keys,
                                        QString const &text);

// Parse a string returned by QKeySequence.toString(), or throw XFormat.
// The returned object is guaranteed to have at least one key.
QKeySequence parseKeySequence(string const &keys);

// Convert a string created with QShortcutEvent::key().toString() to an
// event object.  Throws XFormat on error.  Returns an owner pointer.
QShortcutEvent *getShortcutEventFromString(string const &keys);


// Display an unhandled exception error in a message box.  Limits
// itself to showing five dialog boxes.
void unhandledExceptionMsgbox(QWidget *parent, smbase::XBase const &x);


// Show a message box in a modal dialog.  This is the same as
// QMessageBox::information except it doesn't ring the bell or
// show an "i" icon.
void messageBox(QWidget *parent, QString title, QString message);

// Same, but also provide a "details" string.
void messageBox_details(QWidget *parent, QString title,
                        QString message, QString details);


// Show an informational message box, and wait for it to be dismissed,
// but do not block input to the rest of the app.  The dialog stays
// above `parent`, without preventing interaction with it, if that is
// not nullptr.
void modelessMessageBox(
  QWidget * NULLABLE parent,
  QString const &title,
  QString const &message);


// Ask a question with Yes and Cancel buttons.  Return true on Yes.
bool questionBoxYesCancel(QWidget *parent, QString title, QString question);


// Move 'windowToMove' so it is centered on the center of 'targetWindow'.
void centerWindowOnWindow(QWidget *windowToMove, QWidget *targetWindow);


// Remove the "?" button that gets put in the top-right corner of the
// window frame sometimes.  This is only possible on Qt 5.9 or later;
// on older versions this function does nothing.
void removeWindowContextHelpButton(QWidget *window);


// Show, raise, restore (if minimized) and activate (give focus to) a
// window, thus making it the thing the user will interact with.
void showRaiseAndActivateWindow(QWidget *window);


// Get the rectangle that bounds top-level `window`, including the title
// bar and the window manager frame.  This is what
// `window->frameGeometry()` claims to do, but does not do accurately on
// Windows.
QRect getTrueFrameGeometry(QWidget *window);


// Set the frame geometry accurately.
void setTrueFrameGeometry(
  QWidget *window, QRect const &desiredFrameRect);


// Move top-level `window` so that the top-left corner of its window
// manager frame is at `desiredTopLeft`.  This is what `window->move()`
// claims to do, but does not do so accurately on Windows.
void trueMoveWindow(QWidget *window, QPoint desiredTopLeft);


// Copy `newText` to the window system clipboard.  This can fail on
// Windows when another app has the clipboard open; this function
// returns false when that happens.
bool setClipboardReportFailure(string const &newText);


/*
  Like `setClipboardReportFailure`, but if an attempt fails, keep trying
  every 100ms up to the indicated limit, after which return false.  In
  this function, waiting does *not* pump the event queue.

  Interestingly, Qt already has an internal retry mechanism for the case
  of copying *from* the clipboard; see
  `QWindowsClipboardRetrievalMimeData::retrieveDataObject`.
*/
bool setClipboardWithRetry(string const &newText, int maxRetries = 10);


// Set a widget mouse cursor, then restore it on scope exit.
class CursorSetRestore {
public:      // data
  QWidget *m_widget;
  QCursor m_previousCursor;

public:      // funcs
  CursorSetRestore(QWidget *w, QCursor const &newCursor);
  ~CursorSetRestore();
};


// Set the entire app override cursor temporarily.
class OverrideCursorSetRestore {
public:      // data
  explicit OverrideCursorSetRestore(QCursor const &newCursor);
  ~OverrideCursorSetRestore();
};


// Save the `QPainter` state, then restore it on scope exit.
class QPainterSaveRestore {
  NO_OBJECT_COPIES(QPainterSaveRestore);

public:      // data
  QPainter &m_painter;

public:
  QPainterSaveRestore(QPainter &painter);
  ~QPainterSaveRestore();
};


#endif // QTGUIUTIL_H
