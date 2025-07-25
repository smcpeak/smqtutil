// smqtutil/gui-tests.cc
// GUI tests for `smqtutil`.

#include "smbase/dev-warning.h"        // g_abortUponDevWarning
#include "smbase/exc.h"                // xmessage, smbase::XBase
#include "smbase/sm-file-util.h"       // SMFileUtil
#include "smbase/str.h"                // streq
#include "smbase/stringb.h"            // stringb

#include <QApplication>

#include <cstdlib>                     // std::getenv
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

  // We create the `QApplication` here for all tests, but for a GUI
  // test, the individual test function is responsible for calling
  // `app.exec()`.
  QApplication app(argc, argv);

  bool nogui = false;
  char const *testName = NULL;

  for (int i=1; i < argc; ++i) {
    if (streq(argv[i], "-nogui")) {
      nogui = true;
    }
    else {
      if (testName) {
        xmessage(stringb(
          "Bad argument " << doubleQuote(argv[i]) <<
          ": test name " << doubleQuote(testName) <<
          " already specified."));
      }
      testName = argv[i];
    }
  }

  bool printUsage = !testName && !nogui;

  bool ranOne = false;

  // List of module names for use with `printUsage`.
  std::vector<std::string> moduleNames;

  // Run the test if it is enabled.
  #define RUN_TEST(name)                                          \
    if (printUsage) {                                             \
      moduleNames.push_back(#name);                               \
    }                                                             \
    else if (testName == NULL || streq(testName, #name)) {        \
      if (nogui) {                                                \
        std::cout << "---- " #name " ----" << std::endl;          \
      }                                                           \
      extern void gui_test_##name(QApplication &app, bool nogui); \
      gui_test_##name(app, nogui);                                \
      /* Flush all output streams so that the output */           \
      /* from different tests cannot get mixed up. */             \
      std::cout.flush();                                          \
      std::cerr.flush();                                          \
      ranOne = true;                                              \
    }

  RUN_TEST(qtbdffont);

  #undef RUN_TEST

  if (printUsage) {
    std::cout << R"(Usage:

  ./gui-tests

    Print usage.

  ./gui-tests -nogui

    Runs all nogui tests.

  ./gui-tests -nogui <module>

    Run the nogui tests for <module>.  A "nogui" test is one that is
    not interactive, but still calls functions that are only
    available if Qt can interact with the platform's GUI-capable API
    (such as X11 on unix).

  ./gui-tests <module>

    Run the GUI tests for <module>.  This will pop up something that
    requires user interaction.

Module names:

)";

    for (std::string const &name : moduleNames) {
      std::cout << "  " << name << "\n";
    }

    return;
  }

  if (!ranOne) {
    xmessage(stringb("unrecogized module name: " << testName));
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
