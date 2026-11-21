// silent-mbox-filter-gui-test.cc
// GUI tests for `silent-mbox-filter` module.

#include "unit-tests.h"                // decl for my entry point
#include "silent-mbox-filter.h"        // module under test

// smqtutil
#include "smqtutil/test-main-window.h" // TestMainWindow

// smbase
#include "smbase/sm-env.h"             // smbase::envAsBool
#include "smbase/sm-macros.h"          // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"            // EXPECT_EQ

// Qt
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

using namespace smbase;


// Called from gui-tests.cc.
int gui_test_silent_mbox_filter(QApplication &app, bool nogui)
{
  if (nogui) {
    // This test doesn't have a non-interactive aspect.
    return 0;
  }

  SilentMessageBoxFilter silentFilter(&app);
  if (envAsBool("DISABLE_SILENT")) {
    silentFilter.m_enabled = false;
  }

  TestMainWindow mainWindow("qtguiutil");
  QWidget *widget = new QWidget(&mainWindow);

  {
    QVBoxLayout *vb = new QVBoxLayout();

    #define BUTTON(label)                                         \
    {                                                             \
      QPushButton *btn = new QPushButton("QMessageBox::" #label); \
      QObject::connect(btn, &QPushButton::clicked, [&]() {        \
        QMessageBox::label(widget, #label, #label);               \
      });                                                         \
      vb->addWidget(btn);                                         \
    }

    BUTTON(information)
    BUTTON(question)
    BUTTON(warning)
    BUTTON(critical)

    #undef BUTTON

    vb->addStretch();

    widget->setLayout(vb);
    widget->show();
  }

  mainWindow.setCentralWidget(widget);
  mainWindow.resize(400, 300);
  mainWindow.show();

  // Note: The filter is removed by its own destructor.
  return app.exec();
}


// EOF
