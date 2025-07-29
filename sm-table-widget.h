// sm-table-widget.h
// SMTableWidget class.

#ifndef SMQTUTIL_SM_TABLE_WIDGET_H
#define SMQTUTIL_SM_TABLE_WIDGET_H

#include "sm-table-widget-fwd.h"                 // fwds for this module

#include "smqtutil/col-width-rules.h"            // ColumnWidthRules

// smbase
#include "smbase/sm-noexcept.h"                  // NOEXCEPT
#include "smbase/sm-override.h"                  // OVERRIDE
#include "smbase/std-vector-fwd.h"               // stdfwd::vector

// qt
#include <QString>
#include <QTableWidget>

// libc++
#include <iosfwd>                                // std::ostream
#include <memory>                                // std::unique_ptr

class QModelIndex;


// Choice of minimum or maximum.
//
// TODO: This is a candidate to be moved to smbase.
//
enum Extremum {
  EXTREMUM_MINIMUM,
  EXTREMUM_MAXIMUM
};


// Variant of QTableWidget with some customizations.
//
// Specifically:
//
// * I treat the N and P keys like Down and Up arrow keys for easier
//   keyboard navigation.
//
// * The column sizes are optionally constrained to fill the available
//   horizontal space and respect a set minimum width.
//
class SMTableWidget : public QTableWidget {
  Q_OBJECT

public:      // types
  // Data for initializing a column and specifying its resize behavior.
  class ColumnInfo : public ColumnWidthRules::ColSpec {
  public:      // data
    // User-visible column name.
    QString m_name;

    // Initial column width in pixels.
    int m_initialSize;

  public:      // methods
    ~ColumnInfo();

    ColumnInfo(
      QString const &name,
      int initialSize = 100,
      int minimumSize = 0,
      std::optional<int> maximumSize = std::nullopt,
      int expansionPriority = 1);

    operator gdv::GDValue() const;
  };

public:      // data
  // If true, the columns (try to) fill the available horizontal width;
  // during resize events, the sizes of other columns are adjusted to
  // compensate.  Initially false.
  bool m_columnsFillWidth;

  // If `m_columnsFillWidth`, this is used to calculate column widths
  // during resize events and when the user resizes a column.  Its size
  // should be the same as the number of columns.
  ColumnWidthRules m_colRules;

private:     // funcs
  // Synthesize a keypress for the underlying QTableView.
  void synthesizeKey(int key, Qt::KeyboardModifiers modifiers);

  // Get the sequence of column widths, in pixels.
  stdfwd::vector<int> getColumnWidths() const;

protected Q_SLOTS:
  // React to column resize signals sent by the horizontal header.
  virtual void on_columnResized(
    int logicalIndex, int oldSize, int newSize) NOEXCEPT;

  // React to the selected row changing.
  virtual void on_selectionChanged() NOEXCEPT;

protected:   // funcs
  // If `m_columnsFillWidth`, adjust column widths as needed.
  virtual void resizeEvent(QResizeEvent *event) OVERRIDE;

public:      // funcs
  ~SMTableWidget();
  explicit SMTableWidget(QWidget *parent = nullptr);

  // Configure the table as a list view (items in rows) rather than a
  // control where each cell is separately editable.  Specifically:
  //
  //   - Use Zebra row colors.
  //   - Select rows and groups of rows.
  //   - Remove the corner button.
  //   - Do not use Tab to move among list elements.
  //   - Remove the grid lines.
  //   - Turn off word wrap within cells.
  //
  // You may want to also call `setTextElideMode(Qt::ElideNone)` to
  // turn off "...", which also allows right alignment to work properly.
  void configureAsListView();

  // Set `m_columnsFillWidth` to `b`.  If setting to true, also turn off
  // the horizontal scrollbar.
  void setColumnsFillWidth(bool b);

  // Set the column details.
  void setColumnInfo(stdfwd::vector<ColumnInfo> const &columnInfo);

  // Set the height of `row` to the natural text height, including
  // leading, of the current widget font.  This has to be done for every
  // row separately each time the table is populated.
  void setNaturalTextRowHeight(int row);

  // Adjust the column widths so they fill the widget.  This is done
  // automatically if `m_columnsFillWidth`, but could be done explicitly
  // without that flag.
  void adjustColumnsToFitWidth();

  // Scroll the table horizontally by the specified number of pixels.
  void scrollTableHorizontallyBy(int delta);

  // Scroll to the left or right end.
  void scrollTableHorizontallyToExtremum(Extremum ex);

  // Adjust the vertical scroll position so `row` is visible.  This is
  // done automatically in response to selection changes, but the client
  // can also do it explicitly.
  //
  // Requires: 0 <= row && row < rowCount()
  void scrollToRow(int row);

  // Overridden QWidget methods.
  virtual void keyPressEvent(QKeyEvent *event) NOEXCEPT OVERRIDE;
};


// Print 'index' for debug/log purposes.
std::ostream& operator<< (std::ostream &os, QModelIndex const &index);


#endif // SMQTUTIL_SM_TABLE_WIDGET_H
