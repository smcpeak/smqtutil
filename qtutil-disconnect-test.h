// qtutil-disconnect-test.h
// Test class declarations for qtutil-disconnect-test.cc.

// These classes replicate the `CommandRunner` scenario described in
// doc/signals-and-dtors.txt.

#ifndef SMQTUTIL_QTUTIL_DISCONNECT_TEST_H
#define SMQTUTIL_QTUTIL_DISCONNECT_TEST_H

#include <QObject>

#include <string>                      // std::string


// I cannot use my `OPEN_NAMESPACE` macro because `moc` cannot see into
// it.
namespace qtutil_disconnect_test {


// Plays the role of `QString` (`m_errorMessage`) in the scenario: a
// value that must not be used after its destructor has run.
class ValidityCheckedString {
private:     // data
  // True only between the end of the constructor and the start of
  // the destructor.
  bool m_valid;

  // The value.
  std::string m_value;

public:      // methods
  ValidityCheckedString();
  ~ValidityCheckedString();

  // Each of these asserts `m_valid`.
  void set(std::string const &value);
  std::string get() const;
};


// Plays the role of `QProcess` (`m_process`): an object whose
// destructor can emit a signal.
class DtorSignalEmitter : public QObject {
  Q_OBJECT

public:      // methods
  DtorSignalEmitter();

  // Emits `signal_errorOccurred`, much as `~QProcess` can when it
  // kills its child process.
  virtual ~DtorSignalEmitter() override;

  // Emit the signal now.
  void emitErrorOccurred();

Q_SIGNALS:
  void signal_errorOccurred();
};


// Plays the role of `CommandRunner`.
class Runner : public QObject {
  Q_OBJECT

public:      // class data
  // Number of times `on_errorOccurred` has been called, across all
  // instances.
  static int s_errorOccurredCalls;

public:      // data
  // Declaration order matters: `m_errorMessage` is declared after
  // `m_process`, so it is destroyed *before* `m_process`, whose
  // destructor emits a signal connected to `this`.
  DtorSignalEmitter m_process;

  ValidityCheckedString m_errorMessage;

public:      // methods
  // Connects `m_process` to `this`.
  Runner();

  // Calls `disconnectSignals`, as the policy requires.
  virtual ~Runner() override;

public Q_SLOTS:
  // Writes `m_errorMessage`, analogous to `CommandRunner::setFailed`.
  void on_errorOccurred() noexcept;
};


} // namespace qtutil_disconnect_test


#endif // SMQTUTIL_QTUTIL_DISCONNECT_TEST_H
