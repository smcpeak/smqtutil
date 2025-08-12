// input-dialog-gui-test.cc
// Run `QInputDialog` to check for "Unable to set geometry" error.

#include "smqtutil/qtutil.h"           // doubleQuote(QString)

#include <QInputDialog>
#include <QLineEdit>

#include <iostream>                    // std::cout


// Called from gui-tests.cc.
int gui_test_input_dialog(QApplication &app, bool nogui)
{
  if (nogui) {
    return 0;
  }

  /* This is a fairly ordinary invocation of this dialog, but it
     provokes a useless warning like:

      setGeometry: Unable to set geometry 640x480+860+475 (frame:
      656x519+852+444) on QWidgetWindow/"QInputDialogClassWindow" on
      "\\.\DISPLAY1". Resulting geometry: 640x90+860+475 (frame:
      656x129+852+444) margins: 8, 31, 8, 8 minimum size: 178x90 maximum
      size: 524287x90 MINMAXINFO maxSize=0,0 maxpos=0,0 mintrack=194,129
      maxtrack=524303,129)

    This test is primarily about suppressing that warning, which is done
    by calling `installSMQtUtilMessageHandler()` (in `gui-tests.cc`).
    See comments in/near that function for a little more background.
  */
  bool ok;
  QString text = QInputDialog::getText(nullptr,
    "Connect",
    "SSH Host Name:",
    QLineEdit::Normal,
    "",
    &ok);

  if (ok) {
    std::cout << "Dialog result: " << doubleQuote(text) << "\n";
  }
  else {
    std::cout << "Dialog canceled.\n";
  }

  return 0;
}


// EOF
