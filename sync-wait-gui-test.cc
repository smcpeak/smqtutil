// sync-wait-gui-test.cc
// Tests for `sync-wait` module.

#include "sync-wait-gui-test.h"        // decls for this module

#include "smqtutil/qstringb.h"         // qstringb
#include "smqtutil/sync-wait.h"        // module under test

#include "smbase/exc.h"                // GENERIC_CATCH_BEGIN

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>

#include <iostream>                    // std::cout


SyncWaitTestWindow::SyncWaitTestWindow()
  : TestMainWindow("Sync Wait Test"),
    m_centralWidget(nullptr),
    m_dialogDelayMS(nullptr),
    m_completionTimeMS(nullptr),
    m_widgetPointer(nullptr),
    m_resultLabel(nullptr),
    m_timer(),
    m_timesChecked(0),
    m_waiting(false)
{
  m_centralWidget = new QWidget(this);

  QVBoxLayout *vbox = new QVBoxLayout();

  QPushButton *startButton = new QPushButton("Start");
  QObject::connect(startButton, &QPushButton::pressed,
                   this, &SyncWaitTestWindow::slot_start);
  vbox->addWidget(startButton);

  {
    QHBoxLayout *hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel("Dialog delay MS"));

    m_dialogDelayMS = new QLineEdit("1000");
    hbox->addWidget(m_dialogDelayMS);

    vbox->addLayout(hbox);
  }

  {
    QHBoxLayout *hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel("Completion time MS"));

    m_completionTimeMS = new QLineEdit("2000");
    hbox->addWidget(m_completionTimeMS);

    vbox->addLayout(hbox);
  }

  m_widgetPointer = new QCheckBox(
    "Pass widget pointer (otherwise pass nullptr)");
  m_widgetPointer->setChecked(true);
  vbox->addWidget(m_widgetPointer);

  m_resultLabel = new QLabel("Result: none yet");
  vbox->addWidget(m_resultLabel);

  vbox->addStretch(1);

  m_centralWidget->setLayout(vbox);

  setCentralWidget(m_centralWidget);
}


bool SyncWaitTestWindow::waitCondition()
{
  ++ m_timesChecked;

  bool ret = !m_timer.isActive();
  if (false) {
    std::cout << "waitCondition: times=" << m_timesChecked
              << ", satisfied: " << ret << "\n";
  }
  return ret;
}


void SyncWaitTestWindow::slot_start() noexcept
{
  GENERIC_CATCH_BEGIN

  if (m_waiting) {
    std::cout << "pressed while already waiting, ignoring\n";
    return;
  }

  int delayMS = m_dialogDelayMS->text().toInt();
  int completionMS = m_completionTimeMS->text().toInt();

  // Experimentally, the effect of this is, when true, to prevent
  // interaction with the main window while the activity dialog is open.
  //
  // When false, not only can the user interact with the window while
  // the dialog is open, but any UI actions attempted during the first
  // phase (no dialog) get delivered to the main window once the second
  // phase (the dialog is open) starts.
  bool widgetPointer = m_widgetPointer->isChecked();

  std::cout << "pressed: delayMS=" << delayMS
            << ", completionMS=" << completionMS
            << ", widgetPointer=" << widgetPointer << "\n";

  m_timer.setSingleShot(true);
  m_timer.start(completionMS);
  m_timesChecked = 0;
  m_waiting = true;

  SynchronousWaiter waiter(widgetPointer? m_centralWidget : nullptr);

  bool completed = waiter.waitUntil(
    [this]() -> bool { return this->waitCondition(); },
    delayMS /*ms*/,
    "Activity Title",
    "Activity Message");

  std::cout << "wait finished, completed=" << completed
            << ", timesChecked=" << m_timesChecked <<"\n";

  m_resultLabel->setText(qstringb(
    "Result: completed=" << completed <<
    ", timesChecked=" << m_timesChecked));

  m_waiting = false;

  GENERIC_CATCH_END
}


int gui_test_sync_wait(QApplication &app, bool nogui)
{
  if (nogui) {
    return 0;
  }

  SyncWaitTestWindow mainWindow;
  mainWindow.resize(600, 400);
  mainWindow.show();

  return app.exec();
}


// EOF
