// silent-mbox-filter.cc
// Code for `silent-mbox-filter` module.

#include "silent-mbox-filter.h"        // this module

// smbase
#include "smbase/sm-macros.h"          // IMEMBFP

// Qt
#include <QApplication>
#include <QEvent>
#include <QMessageBox>
#include <QObject>                     // qobject_cast
#include <QStyle>

// libc++
#include <optional>                    // std::optional


// Map the icon of `mb`, if any, to the corresponding
// QStyle::StandardPixmap value.
static std::optional<QStyle::StandardPixmap> getQMessageBoxIconStyle(
  QMessageBox const *mb)
{
  switch (mb->icon()) {
    #define CASE(label)                      \
      case QMessageBox::label:               \
        return QStyle::SP_MessageBox##label;

    CASE(Information)
    CASE(Warning)
    CASE(Critical)

    // FWIW, on my Windows 11 setup, this one is already silent.
    CASE(Question)

    #undef CASE

    default:
      // There is only one other option, `NoIcon`.
      return std::nullopt;
  }

  // Not reached.
}


bool SilentMessageBoxFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (m_enabled && event->type() == QEvent::Show) {
    if (auto *mb = qobject_cast<QMessageBox*>(obj)) {
      if (std::optional<QStyle::StandardPixmap> sp =
            getQMessageBoxIconStyle(mb)) {
        // Get the usual size of icons.
        int size = mb->style()->pixelMetric(
          QStyle::PM_MessageBoxIconSize, nullptr, mb);

        // Re-set the icon in a way that, evidently, causes the
        // associated sound to be suppressed.
        mb->setIconPixmap(
          mb->style()->standardIcon(*sp, nullptr, mb).
          pixmap(size, size));
      }
    }
  }

  return QObject::eventFilter(obj, event);
}


SilentMessageBoxFilter::SilentMessageBoxFilter(QApplication * NULLABLE app)
  : IMEMBFP(app),
    m_enabled(true)
{
  if (m_app) {
    m_app->installEventFilter(this);
  }
}


SilentMessageBoxFilter::~SilentMessageBoxFilter()
{
  if (m_app) {
    m_app->removeEventFilter(this);
  }
}


// EOF
