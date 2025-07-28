// test-main-window.cc
// Code for `test-main-window` module.

#include "test-main-window.h"          // this module

#include <QKeyEvent>


void TestMainWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  else {
    QMainWindow::keyPressEvent(event);
  }
}


// EOF
