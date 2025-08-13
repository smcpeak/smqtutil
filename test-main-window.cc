// test-main-window.cc
// Code for `test-main-window` module.

#include "test-main-window.h"          // this module

#include "smqtutil/qtutil.h"           // toQString

#include "smbase/sm-env.h"             // smbase::envAsIntOr

#include <QKeyEvent>

using namespace smbase;


void TestMainWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  else {
    QMainWindow::keyPressEvent(event);
  }
}


TestMainWindow::TestMainWindow(char const *title)
  : QMainWindow()
{
  setWindowTitle(toQString(title));

  int fontSize = envAsIntOr(12, "FONT_SIZE");
  QFont font = this->font();
  font.setPointSize(fontSize);
  this->setFont(font);
}


// EOF
