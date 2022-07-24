// sm-line-edit.cc
// Code for sm-line-edit.h.

#include "sm-line-edit.h"              // this module


SMLineEdit::SMLineEdit(QString const &contents, QWidget *parent)
  : QLineEdit(contents, parent),
    m_selectAllUponFocus(true)
{}


SMLineEdit::SMLineEdit(QWidget *parent)
  : QLineEdit(parent),
    m_selectAllUponFocus(true)
{}


SMLineEdit::~SMLineEdit()
{}


void SMLineEdit::focusInEvent(QFocusEvent *e)
{
  // This selects the text.
  //
  // Well, judging by the source, it should not always do that,
  // depending on the reason, but the behavior I see appears to be
  // different.
  //
  // https://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/widgets/qlineedit.cpp?h=dev
  QLineEdit::focusInEvent(e);

  if (!m_selectAllUponFocus) {
    // Unselect it if we do not want that.  This leaves the cursor at
    // the end of the text.
    deselect();
  }
}


// EOF
