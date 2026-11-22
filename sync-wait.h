// sync-wait.h
// `synchronouslyWaitUntil`, to synchronously wait in a GUI context.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_SYNC_WAIT_H
#define SMQTUTIL_SYNC_WAIT_H

#include "sync-wait-fwd.h"             // fwds for this module

#include "smbase/sm-macros.h"          // NULLABLE
#include "smbase/std-string-fwd.h"     // std::string

#include <functional>                  // std::function
#include <optional>                    // std::{nullopt, optional}

class QWidget;


// Interface to a synchronous wait capability.
class SynchronousWaiter {
public:      // methods
  virtual ~SynchronousWaiter();

  /*
    Synchronously wait until `condition` is true, then return true; or
    until the operation is canceled, then return false.

    The nominal UI behavior is this function will first show the user a
    "wait" indicator, such as an hourglass mouse cursor.

    Then, after `activityDialogDelayMS` milliseconds, it pops up a
    dialog with the specified title and message, and a Cancel button.
    If the user presses Cancel, then `waitUntil` returns false.

    If `condition` becomes true at any time before Cancel is pressed,
    then any opened dialog is closed, the wait indicator removed, and
    this function returns true.

    The Qt/OS event queue is serviced while waiting.
  */
  virtual bool waitUntil(
    std::function<bool()> condition,
    int activityDialogDelayMS,
    std::string const &activityDialogTitle,
    std::string const &activityDialogMessage) = 0;
};


// This function implements the GUI behavior described as the nominal
// behavior of `waitUntil`.  The modal activity dialog is shown with
// `widget` as its parent.
bool synchronouslyWaitUntil(
  QWidget * NULLABLE widget,
  std::function<bool()> condition,
  int activityDialogDelayMS,
  std::string const &activityDialogTitle,
  std::string const &activityDialogMessage);


// Implementation of SynchronousWaiter that uses
// `synchronouslyWaitUntil`.
class GUISynchronousWaiter : public SynchronousWaiter {
public:      // data
  // The widget whose window will have input blocked while we wait, if
  // any.
  QWidget * NULLABLE m_widget;

  /* If true, then once the condition becomes true (if it does), pop up
     a message box (separate from the progress dialog) that says the
     operation finished, but holds execution there until the dialog is
     dismissed.

     This is meant for use during testing in order to force a specific
     synchronous wait to take arbitrarily long.

     Initially false.
  */
  bool m_showCompletionDialog;

public:      // methods
  explicit GUISynchronousWaiter(QWidget * NULLABLE widget = nullptr);

  virtual bool waitUntil(
    std::function<bool()> condition,
    int activityDialogDelayMS,
    std::string const &activityDialogTitle,
    std::string const &activityDialogMessage) override;
};


// A mock waiter for testing.  It does not show any UI.
class MockSynchronousWaiter : public SynchronousWaiter {
public:      // data
  // If set, then if this is 0, we will immediately return false from
  // `waitUntil`, simulating a canceled wait.  Otherwise we decrement
  // it and wait.  If it is not set, we just wait.
  //
  // Initial value comes from ctor argument.
  std::optional<int> m_cancelCountdown;

  // If true, then attempting to block will throw an exception.
  //
  // Initially false.
  bool m_disallowWaiting;

  // Number of times `waitUntil` was called.  Initially 0.
  int m_waitUntilCount;

public:      // methods
  explicit MockSynchronousWaiter(
    std::optional<int> cancelCountdown = std::nullopt);

  // This ignores everything but `condition`, waiting indefinitely for
  // it to become false.
  virtual bool waitUntil(
    std::function<bool()> condition,
    int activityDialogDelayMS,
    std::string const &activityDialogTitle,
    std::string const &activityDialogMessage) override;
};


#endif // SMQTUTIL_SYNC_WAIT_H
