// qtguiutil-gui-test.cc
// GUI tests for `qtguiutil` module.

#include "smqtutil/qtguiutil.h"        // module under test

#include "smqtutil/gdvalue-qrect.h"    // toGDValue(QRect)
#include "smqtutil/test-main-window.h" // TestMainWindow

#include "smbase/gdvalue.h"            // for TRACE1_GDVN_EXPRS
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

using namespace gdv;


INIT_TRACE("qtguiutil-gui-test");


static void makeWindow(QString const &title, QRect const &r)
{
  QMainWindow *w = new QMainWindow();
  w->setWindowTitle(title);
  setTrueFrameGeometry(w, r);
  w->show();
}


static void buttonFunc(TestMainWindow &mainWindow)
{
  QRect outerRect = getTrueFrameGeometry(&mainWindow);

  // Subdivide the rectangle into four.
  QPoint center = outerRect.center();
  QRect r0 = outerRect;
  QRect r1 = outerRect;
  QRect r2 = outerRect;
  QRect r3 = outerRect;
  r0.setBottomRight(center);
  r1.setBottomLeft(center + QPoint(1,0));
  r2.setTopRight(center + QPoint(0,1));
  r3.setTopLeft(center + QPoint(1,1));

  TRACE1_GDVN_EXPRS("buttonFunc",
    outerRect,
    center,
    r0, r1, r2, r3);

  makeWindow("w0", r0);
  makeWindow("w1", r1);
  makeWindow("w2", r2);
  makeWindow("w3", r3);
}


// Called from gui-tests.cc.
int gui_test_qtguiutil(QApplication &app, bool nogui)
{
  if (nogui) {
    // This test doesn't have a non-interactive aspect.
    return 0;
  }

  TestMainWindow mainWindow("qtguiutil");

  QWidget *widget = new QWidget(&mainWindow);

  {
    QVBoxLayout *vb = new QVBoxLayout();
    QPushButton *btn = new QPushButton("Make windows");
    QObject::connect(btn, &QPushButton::clicked, [&]() {
      buttonFunc(mainWindow);
    });
    vb->addWidget(btn);
    vb->addStretch();

    widget->setLayout(vb);
    widget->show();
  }

  mainWindow.setCentralWidget(widget);
  mainWindow.resize(400, 300);
  mainWindow.show();
  return app.exec();
}


// EOF
