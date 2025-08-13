// sync-wait.h
// `synchronouslyWaitUntil`, to synchronously wait in a GUI context.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_SYNC_WAIT_H
#define SMQTUTIL_SYNC_WAIT_H

#include <functional>                  // std::function

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
*/
bool synchronouslyWaitUntil(
  QWidget *widget,
  std::function<bool()> condition,
  int activityDialogDelayMS,
  char const *activityDialogTitle,
  char const *activityDialogMessage);


#endif // SMQTUTIL_SYNC_WAIT_H
