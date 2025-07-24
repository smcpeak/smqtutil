// sm-table-widget-test.cc
// Test program for `sm-table-widget`.

#include "sm-table-widget.h"           // module under test

#include "smbase/sm-macros.h"          // TABLESIZE

#include <QApplication>
#include <QMainWindow>
#include <QTableWidgetItem>

#include <iostream>                    // std::cout


#define DIAG(stuff) \
  std::cout << stuff << "\n" /* user ; */


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QMainWindow window;

  SMTableWidget *table = new SMTableWidget(&window);
  table->configureAsListView();

  if (false) {
    QFont font = table->font();
    font.setPointSize(30);
    table->setFont(font);
  }

  // The following code mimics how editor/open-files-dialog.cc populates
  // the table.  I'd like to improve the interface, but first I'll just
  // transfer the existing interface into smqtutil.

  std::vector<SMTableWidget::ColumnInfo> columns = {
    { "A", 200, 100 },
    { "B", 100, 30 },
    { "C", 100, 30 },
  };
  table->setColumnInfo(columns);

  table->setRowCount(10);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  table->setColumnsFillWidth(true);

  // Flags for the items.  The point is to omit Qt::ItemIsEditable.
  Qt::ItemFlags const itemFlags =
    Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  for (int row = 0; row < 10; ++row) {
    for (int col = 0; col < static_cast<int>(columns.size()); ++col) {
      QTableWidgetItem *item = new QTableWidgetItem(
        QString("Item %1,%2").arg(row).arg(col));

      item->setFlags(itemFlags);

      if (col == 2) {
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      }

      table->setItem(row, col, item);
    }

    // Remove the row label.  (The default, a NULL item, renders as a
    // row number, which isn't useful here.)
    table->setVerticalHeaderItem(row, new QTableWidgetItem(""));

    // Apparently I have to set every row's height manually.
    // QTreeView has a 'uniformRowHeights' property, but QListView
    // does not.
    table->setNaturalTextRowHeight(row);
  }

  window.setCentralWidget(table);
  window.resize(600, 400);
  window.show();

  return app.exec();
}



// EOF
