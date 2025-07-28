// sm-table-widget-gui-test.cc
// Test program for `sm-table-widget`.

#include "sm-table-widget.h"           // module under test

#include "smbase/sm-env.h"             // smbase::envAsBool
#include "smbase/sm-macros.h"          // TABLESIZE

#include <QApplication>
#include <QMainWindow>
#include <QTableWidgetItem>

#include <iostream>                    // std::cout

using namespace smbase;


// Called from gui-tests.cc.
int gui_test_sm_table_widget(QApplication &app, bool nogui)
{
  if (nogui) {
    return 0;
  }

  QMainWindow window;

  SMTableWidget *table = new SMTableWidget(&window);
  table->configureAsListView();

  int fontSize = envAsIntOr(12, "FONT_SIZE");
  QFont font = table->font();
  font.setPointSize(fontSize);
  table->setFont(font);

  // `configureAsListView` turns off the grid lines, but sometimes I
  // want to see them.
  if (envAsBool("GRID_LINES")) {
    table->setShowGrid(true);
  }

  // True to test a configuration where we are showing a very wide table
  // and intend that the user scroll, rather than keeping it all visible
  // at once.
  bool veryWide = envAsBool("WIDE");

  // Test doing this before `setColumnInfo`.  In the past, that would
  // lead to an assertion failure.
  if (!veryWide) {
    table->setColumnsFillWidth(true);
  }

  std::vector<SMTableWidget::ColumnInfo> columns = {
    // name init  min  max
    { "A",   200, 100 },
    { "B",   100,  30 },
    { "C",   100,  30 },
    { "D",    50,  50, 100 },
  };
  if (veryWide) {
    columns.push_back({QString("Wide"), 400});
  }
  table->setColumnInfo(columns);

  table->setRowCount(10);
  table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  if (veryWide) {
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  }

  // Globally disable elision, which allows right-alignment to work the
  // way I want.
  table->setTextElideMode(Qt::ElideNone);

  // Flags for the items.  The point is to omit Qt::ItemIsEditable.
  Qt::ItemFlags const itemFlags =
    Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  for (int row = 0; row < 10; ++row) {
    for (int col = 0; col < static_cast<int>(columns.size()); ++col) {
      QTableWidgetItem *item = new QTableWidgetItem(
        QString("Item text at %1,%2").arg(row).arg(col));

      if (col == 4) {
        item->setText(QString("This is a very long message. ").repeated(row+1));
      }

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

  if (veryWide) {
    table->resizeColumnToContents(4);
  }

  table->selectRow(0);

  window.setCentralWidget(table);
  window.resize(600, 400);
  window.show();

  return app.exec();
}



// EOF
