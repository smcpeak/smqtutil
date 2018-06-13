// test-layout.cc
// Experiment with some aspects of Qt layout.

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QWidget w;
  w.resize(400, 300);

  // The idea here is to place some buttons in QBoxLayouts using
  // various combinations of stretch factors to observe their effect.

  // Incidentally, I believe this code does *not* leak any memory.
  // Both the widgets and the layouts get added to the QObject tree,
  // which gets cleaned up when 'w' is destroyed.

  QPushButton *b1 = new QPushButton("One");
  QPushButton *b2 = new QPushButton("Two");
  QPushButton *b3 = new QPushButton("Three");
  QPushButton *b4 = new QPushButton("Four");
  QPushButton *b5 = new QPushButton("Five");

  {
    QVBoxLayout *vb = new QVBoxLayout();

    vb->addWidget(new QLabel("One stretch(2) Two stretch(1) Three"));
    {
      QHBoxLayout *hb = new QHBoxLayout();
      hb->addWidget(b1);
      hb->addStretch(2);
      hb->addWidget(b2);
      hb->addStretch(1);
      hb->addWidget(b3);
      vb->addLayout(hb);
    }

    // Interestingly, QLabel has some intrinsic stretchiness both
    // above and below, whereas QPushButton does not.  If I do not
    // add these stretch elements, then the extra vertical space gets
    // added around the labels rather than in the gaps where I want it.
    vb->addStretch(1);

    vb->addWidget(new QLabel("Four Five"));
    {
      QHBoxLayout *hb = new QHBoxLayout();
      hb->addWidget(b4);
      hb->addWidget(b5);
      vb->addLayout(hb);
    }

    vb->addStretch(1);

    vb->addWidget(new QLabel("Six(1) Seven"));
    {
      QHBoxLayout *hb = new QHBoxLayout();
      hb->addWidget(new QPushButton("Six"), 1);
      hb->addWidget(new QPushButton("Seven"));
      vb->addLayout(hb);
    }

    vb->addStretch(1);

    vb->addWidget(new QLabel("Eight(1) Nine(2)"));
    {
      QHBoxLayout *hb = new QHBoxLayout();
      hb->addWidget(new QPushButton("Eight"), 1);
      hb->addWidget(new QPushButton("Nine"), 2);
      vb->addLayout(hb);
    }

    w.setLayout(vb);
  }

  w.show();
  return app.exec();
}


// EOF
