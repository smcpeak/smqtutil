// sm-table-widget.cc
// code for sm-table-widget.h

#include "smbase/gdvalue-vector-fwd.h" // gdv::toGDValue(std::vector)

#include "sm-table-widget.h"           // this module

#include "smqtutil/col-width-rules.h"  // ColumnWidthRules
#include "smqtutil/gdvalue-qstring.h"  // gdv::toGDValue(QString)
#include "smqtutil/qtguiutil.h"        // keysString(QKeyEvent)

#include "smbase/exc.h"                // GENERIC_CATCH_BEGIN/END
#include "smbase/gdvalue-vector.h"     // gdv::toGDValue(std::vector)
#include "smbase/gdvalue.h"            // gdv::GDValue
#include "smbase/ordered-map-ops.h"    // GDVOrderedMap
#include "smbase/sm-macros.h"          // IMEMBFP
#include "smbase/sm-trace.h"           // INIT_TRACE, etc.
#include "smbase/vector-util.h"        // vecSum, vecSlice, vecSumSlice

#include <QFontMetrics>
#include <QHeaderView>
#include <QKeyEvent>
#include <QModelIndex>
#include <QSignalBlocker>

#include <iostream>                    // std::ostream
#include <vector>                      // std::vector


using namespace gdv;


INIT_TRACE("sm-table-widget");


// ---------------------------- ColumnInfo -----------------------------
SMTableWidget::ColumnInfo::~ColumnInfo()
{}


SMTableWidget::ColumnInfo::ColumnInfo(
  QString const &name,
  int initialSize,
  int minimumSize,
  std::optional<int> maximumSize,
  int expansionPriority)
  : ColumnWidthRules::ColSpec(
      minimumSize,
      maximumSize,
      expansionPriority),
    IMEMBFP(name),
    IMEMBFP(initialSize)
{
  selfCheck();
}


SMTableWidget::ColumnInfo::operator gdv::GDValue() const
{
  GDValue m = ColumnWidthRules::ColSpec::operator GDValue();
  m.taggedContainerSetTag("ColumnInfo"_sym);
  GDV_WRITE_MEMBER_SYM(m_name);
  GDV_WRITE_MEMBER_SYM(m_initialSize);
  return m;
}


// --------------------------- SMTableWidget ---------------------------
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


std::vector<int> SMTableWidget::getColumnWidths() const
{
  QHeaderView const *header = horizontalHeader();
  int numColumns = header->count();

  std::vector<int> ret(numColumns);
  for (int i = 0; i < numColumns; ++i) {
    ret[i] = header->sectionSize(i);
  }

  return ret;
}


void SMTableWidget::forceLastColumnToViewportEdge()
{
  QHeaderView * const header = horizontalHeader();
  int const numColumns = header->count();
  int const lastColumnIndex = numColumns - 1;

  std::vector<int> const sizes = getColumnWidths();
  int const beforeColumnsSize = vecSumSlice(sizes, 0, lastColumnIndex);

  int const availSpaceTotal = viewport()->width();
  int newColumnSize =
    m_colRules.clampColumnSize(lastColumnIndex,
      availSpaceTotal - beforeColumnsSize);

  TRACE2("  forcing last column to edge with size " << newColumnSize);

  QSignalBlocker blocker(header);
  header->resizeSection(lastColumnIndex, newColumnSize);
}


void SMTableWidget::on_columnResized(
  int const primaryColumnIndex,
  int const oldPrimarySize,
  int const newPrimarySize) NOEXCEPT
{
  GENERIC_CATCH_BEGIN

  TRACE2("on_columnResized: column=" << primaryColumnIndex <<
         " oldSize=" << oldPrimarySize <<
         " newSize=" << newPrimarySize);

  if (!m_columnsFillWidth) {
    // Use default column sizing behavior.
    return;
  }

  QHeaderView * const header = horizontalHeader();
  int const numColumns = header->count();

  xassert(0 <= primaryColumnIndex && primaryColumnIndex < numColumns);
  xassert(m_colRules.numColumns() == numColumns);

  if (primaryColumnIndex == numColumns-1) {
    // It is possible for the user to click and drag the right edge of
    // the last column.  Force its edge to stay with the viewport right
    // edge.
    forceLastColumnToViewportEdge();
    return;
  }

  // Get current sizes.  This already reflects `newPrimarySize`.
  std::vector<int> const sizes = getColumnWidths();
  xassert(sizes[primaryColumnIndex] == newPrimarySize);

  // Adjust `newPrimarySize` to the declared column bounds.
  int /*not const*/ adjNewPrimarySize =
    m_colRules.clampColumnSize(primaryColumnIndex, newPrimarySize);

  // Get the later columns' widths into a separate vector.
  int const nextColumnIndex = primaryColumnIndex+1;
  std::vector<int> /*not const*/ afterSizes =
    vecSlice(sizes, nextColumnIndex);

  // Space used by the columns before `primaryColumnIndex`.
  int const beforeColumnsSize = vecSumSlice(sizes, 0, primaryColumnIndex);

  // How much space is there for columns after `primaryColumnIndex`?
  int const availSpaceTotal = viewport()->width();
  int const availSpaceAfter =
    availSpaceTotal - beforeColumnsSize - adjNewPrimarySize;

  // Adjust the after columns' sizes to fit.
  m_colRules.resizeSome(
    nextColumnIndex, afterSizes /*INOUT*/, availSpaceAfter);

  // New total size of the after columns.
  int const afterColumnsSize = vecSum(afterSizes);

  if (beforeColumnsSize + adjNewPrimarySize + afterColumnsSize >
                                                      availSpaceTotal) {
    // Don't allow the primary to push the others out of range.
    adjNewPrimarySize =
      m_colRules.clampColumnSize(primaryColumnIndex,
        availSpaceTotal - beforeColumnsSize - afterColumnsSize);
  }

  TRACE3(GDValue(GDVOrderedMap{
    GDV_SKV_EXPR(sizes),
    GDV_SKV_EXPR(adjNewPrimarySize),
    GDV_SKV_EXPR(afterSizes),
    GDV_SKV_EXPR(beforeColumnsSize),
    GDV_SKV_EXPR(availSpaceTotal),
    GDV_SKV_EXPR(availSpaceAfter),
    GDV_SKV_EXPR(afterColumnsSize),
  }).asIndentedString());

  // Apply the changes.
  {
    QSignalBlocker blocker(header);

    // First, the primary column.
    if (adjNewPrimarySize != newPrimarySize) {
      TRACE2("  changed focus column " << primaryColumnIndex <<
             " from " << newPrimarySize <<
             " to " << adjNewPrimarySize);
      header->resizeSection(primaryColumnIndex, adjNewPrimarySize);
    }

    // Then the columns that come after.
    for (int i = nextColumnIndex; i < numColumns; ++i) {
      int const computedSize = afterSizes.at(i - nextColumnIndex);
      if (computedSize != sizes.at(i)) {
        TRACE2("  changed after column " << i <<
               " from " << sizes.at(i) <<
               " to " << computedSize);
        header->resizeSection(i, computedSize);
      }
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

    // Turn off all column minimum sizes.
    QHeaderView * const header = horizontalHeader();
    header->setMinimumSectionSize(0);
  }
}


void SMTableWidget::setColumnInfo(
  std::vector<ColumnInfo> const &columnInfo)
{
  int const numColumns = safeToInt(columnInfo.size());

  // Start by copying the column configuration into `m_colRules` so we
  // are prepared if/when a relevant signal is received.
  m_colRules.m_colSpecs.clear();
  for (int i=0; i < numColumns; i++) {
    m_colRules.m_colSpecs.push_back(columnInfo.at(i));
  }

  // Set the number of table columns.
  setColumnCount(numColumns);

  // Set header labels.
  QStringList columnLabels;
  for (int i=0; i < numColumns; i++) {
    columnLabels << columnInfo.at(i).m_name;
  }
  setHorizontalHeaderLabels(columnLabels);

  // Set column widths to their initial values.
  {
    // Don't react to these size updates.
    QSignalBlocker blocker(horizontalHeader());

    for (int i=0; i < numColumns; i++) {
      setColumnWidth(i, columnInfo.at(i).m_initialSize);
    }
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

  int numColumns = header->count();
  if (numColumns < 1) {
    TRACE2("adjustColumns: less than one column");
    return;
  }
  xassert(numColumns == m_colRules.numColumns());

  // Width of the area inside any scrollbars.
  int viewWidth = viewport()->width();

  // Start with current sizes.
  std::vector<int> curSizes = getColumnWidths();

  // Choose new sizes.
  std::vector<int> newSizes = curSizes;
  if (m_colRules.resizeAll(newSizes, viewWidth)) {
    QSignalBlocker blocker(header);
    for (int i=0; i < numColumns; ++i) {
      if (newSizes[i] != curSizes[i]) {
        TRACE2("changed column " << i << " width from " <<
               curSizes[i] << " to " << newSizes[i]);
        header->resizeSection(i, newSizes[i]);
      }
    }
  }
  else {
    TRACE2("adjustColumns: already have proper width");
  }
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
