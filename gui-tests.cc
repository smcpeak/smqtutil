// smqtutil/gui-tests.cc
// GUI tests for `smqtutil`.

#include "smqtutil/qtutil.h"           // installSMQtUtilMessageHandler

#include "smbase/dev-warning.h"        // g_abortUponDevWarning
#include "smbase/exc.h"                // xmessage, smbase::XBase
#include "smbase/sm-file-util.h"       // SMFileUtil
#include "smbase/str.h"                // streq
#include "smbase/stringb.h"            // stringb

#include <QApplication>

#include <cstdlib>                     // std::{exit, getenv}
#include <exception>                   // std::exception

using namespace smbase;


// This must be `static`, rather than in an anonymous namespace, because
// of the `extern` declarations inside it.
static void entry(int argc, char **argv)
{
  // This is a really ugly way to detect a dependence on X11, and is
  // wrong on Mac OS/X.  But I sunk at least half an hour trying to
  // figure out the proper placement for Q_WS_X11 in Qt5 and could not!
  if (!SMFileUtil().windowsPathSemantics() &&
      !std::getenv("DISPLAY")) {
    std::cout << "Running on non-Windows platform, DISPLAY not set.\n";
    std::cout << "Set DISPLAY in order to run this test.\n";

    // I do not error out here so that in `Makefile` I do not have to
    // try to detect when the test can run.
    return;
  }

  // Suppress "Unable to set geometry" warning.  Without this, the
  // `input_dialog` GUI test would provoke it.
  installSMQtUtilMessageHandler();

  // We create the `QApplication` here for all tests, but for a GUI
  // test, the individual test function is responsible for calling
  // `app.exec()` and returning its exit code.
  QApplication app(argc, argv);

  bool nogui = false;
  bool all = false;
  char const *testName = NULL;

  for (int i=1; i < argc; ++i) {
    char const *arg = argv[i];

    if (streq(arg, "-nogui")) {
      nogui = true;
    }
    else if (streq(arg, "-all")) {
      all = true;
    }
    else if (arg[0] == '-') {
      xmessage(stringb("Unknown option: " << doubleQuote(arg)));
    }
    else {
      if (testName) {
        xmessage(stringb(
          "Bad argument " << doubleQuote(arg) <<
          ": test name " << doubleQuote(testName) <<
          " already specified."));
      }
      testName = arg;
    }
  }

  if (testName && all) {
    xmessage("Cannot combine -all with a module name.");
  }

  bool printUsage = !testName && !all;

  bool ranOne = false;

  // List of module names for use with `printUsage`.
  std::vector<std::string> moduleNames;

  // Run the test if it is enabled.
  #define RUN_TEST(name)                                         \
    if (printUsage) {                                            \
      moduleNames.push_back(#name);                              \
    }                                                            \
    else if (all || streq(testName, #name)) {                    \
      if (nogui) {                                               \
        std::cout << "---- " #name " ----" << std::endl;         \
      }                                                          \
      extern int gui_test_##name(QApplication &app, bool nogui); \
      if (int exitCode = gui_test_##name(app, nogui)) {          \
        std::cout << #name " exit code " << exitCode << "\n";    \
        std::exit(exitCode);                                     \
      }                                                          \
      /* Flush all output streams so that the output */          \
      /* from different tests cannot get mixed up. */            \
      std::cout.flush();                                         \
      std::cerr.flush();                                         \
      ranOne = true;                                             \
    }

  RUN_TEST(input_dialog);
  RUN_TEST(layout);
  RUN_TEST(qtbdffont);
  RUN_TEST(sm_table_widget);

  #undef RUN_TEST

  if (printUsage) {
    std::cout << R"(Usage:

  ./gui-tests [-all] [-nogui] [<module>]

    Without arguments, print usage (this message).

    -all: Run all tests.  Otherwise, exactly one <module> must be
    specified.

    -nogui: Run only the non-interactive, non-GUI parts of the specified
    test(s).  Although these tests are not interactive, they still only
    do something useful if Qt can interact with the platform's Windowing
    API (such as X11 on unix).

    <module>: Run the specified test.

  Without -nogui, this program will pop up a window (one for each test,
  if -all) that at least requires the user to close it (press Esc).

Module names:

)";

    for (std::string const &name : moduleNames) {
      std::cout << "  " << name << "\n";
    }

    return;
  }

  if (!ranOne) {
    xmessage(stringb("unrecogized module name: " << doubleQuote(testName)));
  }

  if (nogui) {
    if (testName) {
      std::cout << "nogui tests for module " << testName << " PASSED\n";
    }
    else {
      std::cout << "All \"gui-tests -nogui\" tests PASSED\n";
    }
  }
}


int main(int argc, char *argv[])
{
  g_abortUponDevWarning = true;
  try {
    entry(argc, argv);
    return 0;
  }
  catch (XBase &x) {
    cerr << x.what() << endl;
    return 2;
  }
  catch (std::exception &x) {
    // Some of the std exceptions are not very self-explanatory without
    // also seeing the exception type.  This is ugly because `name()`
    // returns a mangled name, so I'd like to avoid ever allowing such
    // exceptions to propagate.
    cerr << typeid(x).name() << ": " << x.what() << endl;
    return 2;
  }
}


// EOF
