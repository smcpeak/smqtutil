// sync-wait.cc
// Code for `sync-wait` module.

#include "sync-wait.h"                 // this module

#include "smqtutil/qtguiutil.h"        // OverrideCursorSetRestore
#include "smqtutil/qtutil.h"           // waitForQtEvent

#include "smbase/sm-trace.h"           // INIT_TRACE, etc.

#include <functional>                  // std::function
#include <string>                      // std::string

#include <QApplication>
#include <QCursor>
#include <QProgressDialog>
#include <QTimer>
#include <QWidget>


INIT_TRACE("sync-wait");


// TODO: For the reasons explained in comments on `waitForQtEvent`, it
// would be ideal to have an alternative interface that uses a signal
// instead of the "wait for" loop.
bool synchronouslyWaitUntil(
  QWidget * NULLABLE widget,
  std::function<bool()> condition,
  int activityDialogDelayMS,
  std::string const &activityDialogTitle,
  std::string const &activityDialogMessage)
{
  TRACE1("synchronouslyWaitUntil starting, title: " <<
         activityDialogTitle);

  // Phase 1: No dialog.
  {
    // During the initial wait, we do not accept any input.
    OverrideCursorSetRestore ocsr{QCursor(Qt::WaitCursor)};

    QTimer timer;
    timer.setSingleShot(true);
    timer.start(activityDialogDelayMS /*msec*/);

    while (!condition() && timer.isActive()) {
      // Pump the event queue but defer processing input events.
      TRACE2("synchronouslyWaitUntil: phase 1 wait");

      // Exclude user input during the first phase so the user does not,
      // for example, initiate a second long-duration process, or
      // close the window this one was started in.
      waitForQtEvent(false /*processInputEvents*/);
    }
  }

  bool canceled = false;

  // Phase 2: Progress dialog.
  if (!condition()) {
    TRACE1("synchronouslyWaitUntil phase 2, title: " <<
           activityDialogTitle);

    // The "busy" cursor is a pointer with an hourglass or similar, to
    // indicate we are doing something, but will accept input.
    OverrideCursorSetRestore ocsr{QCursor(Qt::BusyCursor)};

    QProgressDialog progress(
      toQString(activityDialogMessage),          // labelText
      "Cancel",                                  // cancelButtonText,
      0, 1,                                      // minimum and maximum,
      widget);                                   // parent
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0 /*ms*/);
    progress.setWindowTitle(toQString(activityDialogTitle));

    // The minimum, maximum, and value do not make sense in this
    // situation, but I don't think I can just remove them.
    progress.setValue(0);

    while (!condition() && !progress.wasCanceled()) {
      TRACE2("synchronouslyWaitUntil: phase 2 wait");

      // This *does* permit user input events, as that is required for
      // the user to be able to press the Cancel button.
      waitForQtEvent(true /*processInputEvents*/);
    }

    if (progress.wasCanceled()) {
      canceled = true;
    }

    // Dismiss the progress dialog.
    progress.reset();

    QGuiApplication::restoreOverrideCursor();
  }

  TRACE1("synchronouslyWaitUntil finishing: canceled=" << canceled <<
         ", title: " << activityDialogTitle);

  return !canceled;
}


// EOF
