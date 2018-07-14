// timer-event-loop.h
// TimerEventLoop class.

#ifndef TIMER_EVENT_LOOP_H
#define TIMER_EVENT_LOOP_H

// smbase
#include "macros.h"                    // NO_OBJECT_COPIES
#include "sm-override.h"               // OVERRIDE

// Qt
#include <QEventLoop>


// An event loop and a timer so I can easily pump the event queue for
// a certain amount of time.
class TimerEventLoop : public QEventLoop {
  NO_OBJECT_COPIES(TimerEventLoop);

private:     // data
  // Running timer or 0 if none.
  int m_timerId;

protected:   // funcs
  // QObject methods.
  virtual void timerEvent(QTimerEvent *Event) OVERRIDE;

  // Stop the timer if it is running.
  void stopTimerIf();

public:      // funcs
  TimerEventLoop()
    : m_timerId(0)
  {}

  // Wait, while pumping the event queue, for 'msecs'.
  void waitForMS(int msecs);
};


// Pause for 'ms' milliseconds, but pump the event queue while waiting.
// The process as a whole does not stop, we just delay some processing.
// Of course, this should not be abused, and at the moment my only
// intended purpose is to use in test code.
void sleepWhilePumpingEvents(int ms);


#endif // TIMER_EVENT_LOOP_H
