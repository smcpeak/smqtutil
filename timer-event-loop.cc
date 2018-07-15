// timer-event-loop.cc
// code for timer-event-loop.h

#include "timer-event-loop.h"          // this module

// smbase
#include "xassert.h"                   // xassert


void TimerEventLoop::timerEvent(QTimerEvent *Event)
{
  // Stop the event loop.
  this->exit(0);

  this->stopTimerIf();
}

void TimerEventLoop::stopTimerIf()
{
  if (m_timerId != 0) {
    this->killTimer(m_timerId);
    m_timerId = 0;
  }
}

void TimerEventLoop::waitForMS(int msecs)
{
  this->stopTimerIf();

  m_timerId = this->startTimer(msecs);
  xassert(m_timerId != 0);

  // Start the event loop.
  this->exec();
}



void sleepWhilePumpingEvents(int ms)
{
  TimerEventLoop eventLoop;
  eventLoop.waitForMS(ms);
}


// EOF
