// qtguiutil.cc
// code for qtguiutil.h

#include "qtguiutil.h"                 // this module

// smqtutil
#include "qtutil.h"                    // toString for Key and Modifiers

// smbase
#include "exc.h"                       // xformat
#include "strutil.h"                   // quoted(string)

// Qt
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QShortcutEvent>
#include <QString>
#include <QStringList>
#include <QWidget>

// libc
#include <string.h>                    // memcmp


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
  catch (xFormat &msg) {
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
  catch (xFormat &msg) {
    xformatsb("in key string " << quoted(keys) << ": " << msg.cond());
  }
}


QShortcutEvent *getShortcutEventFromString(string const &keys)
{
  QKeySequence kseq(parseKeySequence(keys));

  // So far, the replay process appears to not be sensitive to the ID.
  // It would be easy to record it, of course, but I do not think it is
  // stable over time.
  return new QShortcutEvent(kseq, 0 /*id*/);
}


void unhandledExceptionMsgbox(QWidget *parent, xBase const &x)
{
  // Print to stderr as well.
  printUnhandled(x);

  static int count = 0;
  if (++count >= 5) {
    // Stop showing dialog boxes at a certain point.
    return;
  }

  QMessageBox::critical(parent, "Oops",
    QString(stringc << "Unhandled exception: " << x.why() << "\n"
                    << "Save your work if you can!"));
}


void messageBox(QWidget *parent, QString title, QString message)
{
  QMessageBox box(parent);
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


// EOF
