// silent-mbox-filter.h
// `SilentMessageBoxFilter`, to prevent MessageBoxes from making noise.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_SILENT_MBOX_FILTER_H
#define SMQTUTIL_SILENT_MBOX_FILTER_H

// smbase
#include "smbase/sm-macros.h"          // NULLABLE

// Qt
#include <QObject>

class QApplication;
class QEvent;


/*
  When installed on the QApplication, which happens automatically if the
  app is passed to the ctor, this filter suppresses the sounds that
  QMessageBox ordinarily makes.
*/
class SilentMessageBoxFilter : public QObject {
public:      // data
  // The app we are installed into.
  //
  // This can be nullptr to inhibit the install/uninstall behavior.
  QApplication * NULLABLE m_app;

  // True if the filter should silence QMessageBoxes, false to do
  // nothing.  Initially true.
  bool m_enabled;

protected:   // funcs
  virtual bool eventFilter(QObject *obj, QEvent *event) override;

public:      // funcs
  // Install this object into `app` if not nullptr.
  SilentMessageBoxFilter(QApplication * NULLABLE app);

  // Uninstall the filter when destroyed.
  ~SilentMessageBoxFilter();
};


#endif // SMQTUTIL_SILENT_MBOX_FILTER_H
