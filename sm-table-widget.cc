// sm-table-widget.cc
// code for sm-table-widget.h

#include "sm-table-widget.h"           // this module

// smqtutil
#include "smqtutil/qtguiutil.h"        // keysString(QKeyEvent)

// smbase
#include "smbase/exc.h"                // GENERIC_CATCH_BEGIN/END
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.

// Qt
#include <QFontMetrics>
#include <QHeaderView>
#include <QKeyEvent>
#include <QModelIndex>

// libc++
#include <iostream>                    // std::ostream


INIT_TRACE("sm-table-widget");


SMTableWidget::SMTableWidget(QWidget *parent)
  : QTableWidget(parent),
    m_columnsFillWidth(false)
{
  // Pixel granularity scrolling is much smoother than row/col.
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

  QObject::connect(horizontalHeader(), &QHeaderView::sectionResized,
                   this, &SMTableWidget::on_columnResized);
}


SMTableWidget::~SMTableWidget()
{
  QObject::disconnect(horizontalHeader(), nullptr, this, nullptr);
}


void SMTableWidget::synthesizeKey(int key, Qt::KeyboardModifiers modifiers)
{
  QKeyEvent press(QEvent::KeyPress, key, modifiers);
  this->QTableWidget::keyPressEvent(&press);

  // I don't think QTableWidget actually cares about key release events,
  // but this seems like the generally right thing to do.
  QKeyEvent release(QEvent::KeyRelease, key, modifiers);
  this->QTableWidget::keyReleaseEvent(&release);
}


void SMTableWidget::on_columnResized(int logicalIndex, int oldSize, int newSize) NOEXCEPT
{
  GENERIC_CATCH_BEGIN

  TRACE2("on_columnResized: column=" << logicalIndex <<
         " oldSize=" << oldSize <<
         " newSize=" << newSize);

  if (!m_columnsFillWidth) {
    // Use default column sizing behavior.
    return;
  }

  QHeaderView *header = horizontalHeader();
  int columnCount = header->count();

  if (logicalIndex < 0 || logicalIndex >= columnCount - 1)
    return; // Only handle resizing for A and B

  int nextIndex = logicalIndex + 1;

  // The following logic is more convoluted than necessary.  The
  // net effect is that resizing one column always changes the
  // neighbor to the right by the opposite amount, clamping both
  // to a minimum pixel size.
  //
  // TODO: Improve this.

  int minSize = header->minimumSectionSize();
  int nextSize = header->sectionSize(nextIndex);

  int totalWidth = viewport()->width();

  int otherTotal = 0;
  for (int i = 0; i < columnCount; ++i) {
    if (i != nextIndex && i != logicalIndex) {
      otherTotal += header->sectionSize(i);
    }
  }

  // The resized section can be at most:
  int maxThisSectionSize = totalWidth - otherTotal - minSize;

  // Clamp if too big
  if (newSize > maxThisSectionSize) {
    header->blockSignals(true);
    TRACE2("  clamping; setting this section " << logicalIndex <<
           " to size " << maxThisSectionSize <<
           " and next section " << nextIndex <<
           " to size " << minSize);
    header->resizeSection(logicalIndex, maxThisSectionSize);
    header->resizeSection(nextIndex, minSize);
    header->blockSignals(false);
    return;
  }

  // Otherwise adjust neighbor
  int newNextSize = totalWidth - otherTotal - newSize;
  if (newNextSize < minSize) {
    newNextSize = minSize;
    int adjustedThis = totalWidth - otherTotal - minSize;
    header->blockSignals(true);
    TRACE2("  adj both; setting this section " << logicalIndex <<
           " to size " << adjustedThis <<
           " and next section " << nextIndex <<
           " to size " << minSize);
    header->resizeSection(logicalIndex, adjustedThis);
    header->resizeSection(nextIndex, minSize);
    header->blockSignals(false);
  } else {
    if (newNextSize != nextSize) {
      header->blockSignals(true);
      TRACE2("  setting next section " << nextIndex <<
             " to size " << newNextSize);
      header->resizeSection(nextIndex, newNextSize);
      header->blockSignals(false);
    }
  }

  GENERIC_CATCH_END
}


void SMTableWidget::resizeEvent(QResizeEvent *event)
{
  TRACE2("resizeEvent");
  QTableWidget::resizeEvent(event);

  if (m_columnsFillWidth) {
    adjustColumnsToFitWidth();
  }
}


void SMTableWidget::keyPressEvent(QKeyEvent *event) NOEXCEPT
{
  GENERIC_CATCH_BEGIN

  TRACE1("keyPressEvent: " << keysString(*event));

  switch (event->key()) {
    case Qt::Key_N: {
      // We pass along the same modifiers so that the user can do, e.g.,
      // Shift+N to extend the selection, etc.
      this->synthesizeKey(Qt::Key_Down, event->modifiers());
      return;
    }

    case Qt::Key_P: {
      this->synthesizeKey(Qt::Key_Up, event->modifiers());
      return;
    }
  }

  QTableWidget::keyPressEvent(event);

  GENERIC_CATCH_END
}


void SMTableWidget::configureAsListView()
{
  // Zebra table.
  setAlternatingRowColors(true);

  // Select entire rows at a time.
  setSelectionBehavior(QAbstractItemView::SelectRows);

  // Click to select.  Shift+Click to extend contiguously, Ctrl+Click
  // to toggle one element.
  setSelectionMode(QAbstractItemView::ExtendedSelection);

  // Do not respond to clicks in the tiny top-left corner sliver.
  //
  // I do not appear to be able to get rid of the thin left column
  // altogether.
  setCornerButtonEnabled(false);

  // Do not use Tab to move among cells.  Rather, it should move the
  // focus among controls in the dialog.
  setTabKeyNavigation(false);

  // Do not draw grid lines.  They only add visual clutter.
  setShowGrid(false);
}


void SMTableWidget::setColumnsFillWidth(bool b)
{
  m_columnsFillWidth = b;
  if (m_columnsFillWidth) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }
}


void SMTableWidget::initializeColumns(ColumnInitInfo const *columnInfo,
                                      int numColumns)
{
  setColumnCount(numColumns);

  // Header labels.
  QStringList columnLabels;
  for (int i=0; i < numColumns; i++) {
    columnLabels << columnInfo[i].name;
  }
  setHorizontalHeaderLabels(columnLabels);

  // Column widths.
  for (int i=0; i < numColumns; i++) {
    setColumnWidth(i, columnInfo[i].initialWidth);
  }
}


void SMTableWidget::setNaturalTextRowHeight(int row)
{
  // It doesn't matter what value I use here since the table has a
  // minimum row height of `verticalHeader()->minimumSectionSize()`,
  // which is `QFontMetrics(...).height()+8`.
  //
  // But I still have to set *some* height, since otherwise I'm stuck
  // with the default too-large height.

  int height = QFontMetrics(font()).height();
  setRowHeight(row, height);
  TRACE2("after setting height of row " << row << " to " << height <<
         ", it now reports a height of " << rowHeight(row));
}


void SMTableWidget::adjustColumnsToFitWidth()
{
  QHeaderView *header = horizontalHeader();
  int count = header->count();
  if (count < 1)
    return;

  int minSize = header->minimumSectionSize();

  int totalWidth = viewport()->width();

  QVector<int> sizes(count);
  for (int i = 0; i < count; ++i) {
    sizes[i] = header->sectionSize(i);
  }

  int currentTotal = 0;
  for (int size : sizes)
    currentTotal += size;

  if (currentTotal == totalWidth)
    return;

  // Scale all columns proportionally, but respect min size.
  int diff = totalWidth - currentTotal;

  int adjustableColumns = count;
  for (int i = 0; i < count; ++i) {
    if (sizes[i] <= minSize && diff < 0)
      adjustableColumns--;
  }

  if (adjustableColumns <= 0)
    return;

  int deltaPerCol = diff / adjustableColumns;

  header->blockSignals(true);

  for (int i = 0; i < count; ++i) {
    int newSize = sizes[i] + deltaPerCol;
    if (newSize < minSize)
      newSize = minSize;
    header->resizeSection(i, newSize);
  }

  header->blockSignals(false);
}


void SMTableWidget::setMinimumColumnWidth(int width)
{
  horizontalHeader()->setMinimumSectionSize(width);
}


std::ostream& operator<< (std::ostream &os, QModelIndex const &index)
{
  if (!index.isValid()) {
    return os << "root";
  }
  else {
    return os << index.parent()
              << ".(r=" << index.row()
              << ", c=" << index.column() << ')';
  }
}


// EOF
