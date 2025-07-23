// sm-table-widget.h
// SMTableWidget class.

#ifndef SMQTUTIL_SM_TABLE_WIDGET_H
#define SMQTUTIL_SM_TABLE_WIDGET_H

// smbase
#include "smbase/sm-noexcept.h"        // NOEXCEPT
#include "smbase/sm-override.h"        // OVERRIDE

// qt
#include <QString>
#include <QTableWidget>

// libc++
#include <iosfwd>                      // std::ostream

class QModelIndex;


// Variant of QTableWidget with some customizations.
//
// Specifically:
//
// * I treat the N and P keys like Down and Up arrow keys for easier
//   keyboard navigation.
//
// * TODO: The column sizes are constrained to fill the available
//   horizontal space and respect a set minimum width.
//
class SMTableWidget : public QTableWidget {
  Q_OBJECT

public:      // types
  // Data for initializing a column.
  struct ColumnInitInfo {
    QString name;            // User-visible column name.
    int initialWidth;        // Initial column width in pixels.
  };

private:     // funcs
  // Synthesize a keypress for the underlying QTableView.
  void synthesizeKey(int key, Qt::KeyboardModifiers modifiers);

public:      // funcs
  SMTableWidget(QWidget *parent = NULL);
  ~SMTableWidget();

  // Configure the table as a list view (items in rows) rather than a
  // control where each cell is separately editable.  Specifically:
  //   - Use Zebra row colors.
  //   - Select rows and groups of rows.
  //   - Remove the corner button.
  //   - Do not use Tab to move among list elements.
  //   - Remove the grid lines.
  void configureAsListView();

  // Set the column names and initial widths from 'columnInfo', an array
  // of size 'numColumns'.
  void initializeColumns(ColumnInitInfo const *columnInfo,
                         int numColumns);

  // Set the height of `row` to the natural text height, including
  // leading, of the current widget font.  This has to be done for every
  // row separately each time the table is populated.
  void setNaturalTextRowHeight(int row);

  // Overridden QWidget methods.
  virtual void keyPressEvent(QKeyEvent *event) NOEXCEPT OVERRIDE;
};


// Print 'index' for debug/log purposes.
std::ostream& operator<< (std::ostream &os, QModelIndex const &index);


#endif // SMQTUTIL_SM_TABLE_WIDGET_H
