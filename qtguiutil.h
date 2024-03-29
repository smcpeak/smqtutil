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

#include <QCursor>

class QKeyEvent;
class QKeySequence;
class QShortcutEvent;
class QString;
class QWidget;


// Render as a string for debugging or event record/replay.  This
// string just captures the key(s), not the text.
string keysString(QKeyEvent const &k);

// Convert the string back to a QKeyEvent for event replay.  The
// returned pointer is an owner pointer.  Throws xFormat if the string
// cannot be parsed as a KeyPress event.
QKeyEvent *getKeyPressEventFromString(string const &keys,
                                      QString const &text);

// Same, but as a release event.
QKeyEvent *getKeyReleaseEventFromString(string const &keys,
                                        QString const &text);

// Parse a string returned by QKeySequence.toString(), or throw xFormat.
// The returned object is guaranteed to have at least one key.
QKeySequence parseKeySequence(string const &keys);

// Convert a string created with QShortcutEvent::key().toString() to an
// event object.  Throws xFormat on error.  Returns an owner pointer.
QShortcutEvent *getShortcutEventFromString(string const &keys);


// Display an unhandled exception error in a message box.  Limits
// itself to showing five dialog boxes.
void unhandledExceptionMsgbox(QWidget *parent, xBase const &x);


// Show a message box in a modal dialog.  This is the same as
// QMessageBox::information except it doesn't ring the bell or
// show an "i" icon.
void messageBox(QWidget *parent, QString title, QString message);

// Same, but also provide a "details" string.
void messageBox_details(QWidget *parent, QString title,
                        QString message, QString details);


// Ask a question with Yes and Cancel buttons.  Return true on Yes.
bool questionBoxYesCancel(QWidget *parent, QString title, QString question);


// Move 'windowToMove' so it is centered on the center of 'targetWindow'.
void centerWindowOnWindow(QWidget *windowToMove, QWidget *targetWindow);


// Set a widget mouse cursor, then restore it on scope exit.
class CursorSetRestore {
public:      // data
  QWidget *m_widget;
  QCursor m_previousCursor;

public:      // funcs
  CursorSetRestore(QWidget *w, QCursor const &newCursor);
  ~CursorSetRestore();
};


#endif // QTGUIUTIL_H
