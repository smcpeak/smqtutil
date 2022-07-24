// sm-line-edit.h
// SMLineEdit class.

#ifndef SMQTUTIL_SM_LINE_EDIT_H
#define SMQTUTIL_SM_LINE_EDIT_H

#include <QLineEdit>


// QLineEdit with some minor customization.
//
// SMLineEdit contains options I think QLineEdit should have.
// SMLineEdit behaves like ordinary QLineEdit by default, so it should
// be safe to always use it in place of QLineEdit.
class SMLineEdit : public QLineEdit {
public:      // data
  // When true (the default), select all text when the control receives
  // focus.
  bool m_selectAllUponFocus;

protected:   // methods
  virtual void focusInEvent(QFocusEvent *e) override;

public:      // methods
  SMLineEdit(QString const &contents, QWidget *parent = nullptr);
  SMLineEdit(QWidget *parent = nullptr);
  virtual ~SMLineEdit() override;
};


#endif // SMQTUTIL_SM_LINE_EDIT_H
