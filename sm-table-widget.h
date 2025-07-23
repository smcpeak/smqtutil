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

public:      // data
  // If true, the columns (try to) fill the available horizontal width;
  // during resize events, the sizes of other columns are adjusted to
  // compensate.  Initially false.
  bool m_columnsFillWidth;

private:     // funcs
  // Synthesize a keypress for the underlying QTableView.
  void synthesizeKey(int key, Qt::KeyboardModifiers modifiers);

protected Q_SLOTS:
  // React to column resize signals sent by the horizontal header.
  void on_columnResized(int logicalIndex, int oldSize, int newSize) NOEXCEPT;

protected:   // funcs
  // If `m_columnsFillWidth`, adjust column widths as needed.
  virtual void resizeEvent(QResizeEvent *event) OVERRIDE;

public:      // funcs
  SMTableWidget(QWidget *parent);
  ~SMTableWidget();

  // Configure the table as a list view (items in rows) rather than a
  // control where each cell is separately editable.  Specifically:
  //   - Use Zebra row colors.
  //   - Select rows and groups of rows.
  //   - Remove the corner button.
  //   - Do not use Tab to move among list elements.
  //   - Remove the grid lines.
  void configureAsListView();

  // Set `m_columnsFillWidth` to `b`.  If setting to true, also turn off
  // the horizontal scrollbar.
  void setColumnsFillWidth(bool b);

  // Set the column names and initial widths from 'columnInfo', an array
  // of size 'numColumns'.
  void initializeColumns(ColumnInitInfo const *columnInfo,
                         int numColumns);

  // Set the height of `row` to the natural text height, including
  // leading, of the current widget font.  This has to be done for every
  // row separately each time the table is populated.
  void setNaturalTextRowHeight(int row);

  // Adjust the column widths so they fill the widget.  This is done
  // automatically if `m_columnsFillWidth`, but could be done explicitly
  // without that flag.
  void adjustColumnsToFitWidth();

  // Set the minimum column width, in pixels.
  void setMinimumColumnWidth(int width);

  // Overridden QWidget methods.
  virtual void keyPressEvent(QKeyEvent *event) NOEXCEPT OVERRIDE;
};


// Print 'index' for debug/log purposes.
std::ostream& operator<< (std::ostream &os, QModelIndex const &index);


#endif // SMQTUTIL_SM_TABLE_WIDGET_H
