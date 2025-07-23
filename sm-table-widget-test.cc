// sm-table-widget-test.cc
// Test program for `sm-table-widget`.

#include "sm-table-widget.h"           // module under test

#include "smbase/sm-macros.h"          // TABLESIZE

#include <QApplication>
#include <QMainWindow>
#include <QTableWidgetItem>


// Height of each row in pixels.
//
// TODO: This hardcoded value is ugly.  I should instead detect and use
// a value that depends on the font.
int const ROW_HEIGHT = 20;


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QMainWindow window;

  SMTableWidget *table = new SMTableWidget(&window);
  table->configureAsListView();

  // The following code mimics how editor/open-files-dialog.cc populates
  // the table.  I'd like to improve the interface, but first I'll just
  // transfer the existing interface into smqtutil.

  SMTableWidget::ColumnInitInfo columns[] = {
    { "A", 200 },
    { "B", 100 },
    { "C", 100 },
  };
  table->initializeColumns(columns, TABLESIZE(columns));
  table->setRowCount(10);

  // Flags for the items.  The point is to omit Qt::ItemIsEditable.
  Qt::ItemFlags const itemFlags =
    Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  for (int row = 0; row < 10; ++row) {
    // Remove the row label.  (The default, a NULL item, renders as a
    // row number, which isn't useful here.)
    table->setVerticalHeaderItem(row, new QTableWidgetItem(""));

    for (int col = 0; col < TABLESIZE(columns); ++col) {
      QTableWidgetItem *item = new QTableWidgetItem(
        QString("Item %1,%2").arg(row).arg(col));

      item->setFlags(itemFlags);

      if (col == 2) {
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      }

      table->setItem(row, col, item);

      // Apparently I have to set every row's height manually.
      // QTreeView has a 'uniformRowHeights' property, but QListView
      // does not.
      table->setRowHeight(row, ROW_HEIGHT);
    }
  }

  window.setCentralWidget(table);
  window.resize(600, 400);
  window.show();

  return app.exec();
}



// EOF
