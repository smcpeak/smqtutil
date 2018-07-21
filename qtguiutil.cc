// qtguiutil.cc
// code for qtguiutil.h

#include "qtguiutil.h"                 // this module

// smqtutil
#include "qtutil.h"                    // toString for Key and Modifiers

// smbase
#include "exc.h"                       // xformat

// Qt
#include <QKeyEvent>
#include <QMessageBox>
#include <QShortcutEvent>
#include <QStringList>
#include <QWidget>


string toString(QKeyEvent const &k)
{
  return stringc << toString(k.modifiers())
                 << "+" << toString((Qt::Key)(k.key()));
}


QKeyEvent *getKeyPressEventFromString(string const &str)
{
  try {
    QStringList keys(toQString(str).split('+'));
    if (keys.isEmpty()) {
      xformat("no keys in string");
    }

    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    for (int i=0; i < keys.count()-1; i++) {
      modifiers |= getKeyboardModifierFromString(toString(keys.at(i)));
    }

    Qt::Key key = getKeyFromString(toString(keys.last()));

    return new QKeyEvent(QEvent::KeyPress, key, modifiers);
  }
  catch (xFormat &msg) {
    xformatsb("in key string \"" << str << "\": " << msg.cond());
  }
}


QShortcutEvent *getShortcutEventFromString(string const &str)
{
  // I do not know how to check for errors here.
  QKeySequence kseq(QKeySequence::fromString(toQString(str)));

  // I hope the replay process is not sensitive to the ID.  It would be
  // easy to record it, of course, but I do not think it is stable over
  // time.
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
