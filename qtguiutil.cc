// qtguiutil.cc
// code for qtguiutil.h

#include "qtguiutil.h"                 // this module

#include "qtutil.h"                    // toString for Key and Modifiers

#include <QKeyEvent>
#include <QMessageBox>
#include <QWidget>


string toString(QKeyEvent const &k)
{
  return stringc << toString(k.modifiers())
                 << "+" << toString((Qt::Key)(k.key()));
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
