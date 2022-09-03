// qtutil-test.h
// Test class declarations for qtutil-test.cc.

#ifndef SMQTUTIL_QTUTIL_TEST_H
#define SMQTUTIL_QTUTIL_TEST_H

#include <QObject>


// An object that can send a signal.
class Sender : public QObject {
  Q_OBJECT

public:      // methods
  Sender();
  virtual ~Sender() override;

Q_SIGNALS:
  void signal_sig1();
};


// An object that can receive a signal.
class Receiver : public QObject {
  Q_OBJECT

public:      // data
  // Number of time the signal has been received.
  int m_receipts;

public:      // methods
  Receiver();
  virtual ~Receiver() override;

public Q_SLOTS:
  void on_sig1() noexcept;
};


#endif // SMQTUTIL_QTUTIL_TEST_H
