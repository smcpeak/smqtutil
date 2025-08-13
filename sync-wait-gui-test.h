// sync-wait-gui-test.h
// Class decls for `sync-wait-gui-test.cc`.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_SYNC_WAIT_GUI_TEST_H
#define SMQTUTIL_SYNC_WAIT_GUI_TEST_H

#include "smqtutil/test-main-window.h" // TestMainWindow

#include <QTimer>

class QLabel;
class QLineEdit;
class QWidget;


class SyncWaitTestWindow : public TestMainWindow {
public:      // data
  // Widget set as central via `setCentralWidget`.
  QWidget *m_centralWidget;

  // Time to wait before showing the dialog.
  QLineEdit *m_dialogDelayMS;

  // Time until completion.
  QLineEdit *m_completionTimeMS;

  // Result of the most recent wait session.
  QLabel *m_resultLabel;

  // Timer for when condition is satisfied.
  QTimer m_timer;

  // Number of times the condition has been checked since we started the
  // current wait.
  int m_timesChecked;

  // True if we are the middle of a wait.
  bool m_waiting;

public:      // methods
  SyncWaitTestWindow();

  // True if the relevant condition is true.
  bool waitCondition();

public Q_SLOTS:
  void slot_start() noexcept;
};


#endif // SMQTUTIL_SYNC_WAIT_GUI_TEST_H
