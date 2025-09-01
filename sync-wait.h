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


/* Synchronously wait until `condition` is true.

   For the first `activityDialogDelayMS` milliseconds, show a "wait"
   mouse cursor in `widget`, while blocking input to it as if a modal
   dialog was shown.  During this time, if the condition becomes true,
   restore the mouse cursor and return true.

   After that time elapses, restore the mouse cursor and pop up a modal
   activity dialog with the specified title and message, and containing
   a Cancel button.  During this time, if the condition becomes true,
   close the dialog and return true.  If the user presses Cancel, close
   the dialog and return false.

   If `widget` is non-null, then user input to its window is disabled
   for the duration of this function.
*/
bool synchronouslyWaitUntil(
  QWidget * NULLABLE widget,
  std::function<bool()> condition,
  int activityDialogDelayMS,
  std::string const &activityDialogTitle,
  std::string const &activityDialogMessage);


// Interface to a synchronous wait capability.
class SynchronousWaiter {
public:      // data
  // The widget whose window will have input blocked while we wait, if
  // any.
  QWidget * NULLABLE m_widget;

public:      // methods
  explicit SynchronousWaiter(QWidget *widget = nullptr);

  // Call `synchronouslyWaitUntil`, returning its result.
  //
  // However, this is meant to be overridden for testing purposes.
  virtual bool waitUntil(
    std::function<bool()> condition,
    int activityDialogDelayMS,
    std::string const &activityDialogTitle,
    std::string const &activityDialogMessage);
};


// A mock waiter for testing.  It does not show any UI.
class TestSynchronousWaiter : public SynchronousWaiter {
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
  explicit TestSynchronousWaiter(
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
