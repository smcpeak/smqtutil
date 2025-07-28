// test-main-window.h
// `TestMainWindow`, a main window for use in tests.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_TEST_MAIN_WINDOW_H
#define SMQTUTIL_TEST_MAIN_WINDOW_H

#include <QMainWindow>


// A main window for use in tests.
//
// For the moment, the only functionality it provides is reacting to the
// Esc key by closing.
//
class TestMainWindow : public QMainWindow {
protected:   // methods
  virtual void keyPressEvent(QKeyEvent *event) override;
};


#endif // SMQTUTIL_TEST_MAIN_WINDOW_H
