// qhboxframe.h
// Define QHBoxFrame class.

#ifndef QHBOXFRAME
#define QHBOXFRAME

#include <QHBoxLayout>
#include <QFrame>


// QFrame with an HBox layout.
class QHBoxFrame : public QFrame {
public:      // data
  // Layout for this frame.  It is the same as layout(),
  // but with the more specific type.  This is not an
  // owner pointer per se, but the containing object does
  // own the layout due to setLayout().
  //
  // This layout is set with 0 spacing and margins, unlike
  // a default-constructed QHBoxLayout.
  QHBoxLayout *hbox;

public:      // funcs
  QHBoxFrame(QWidget *parent = NULL, Qt::WindowFlags f = Qt::WindowFlags());
  ~QHBoxFrame();

  // Add the 'w' to the layout and to this object as a
  // child widget.
  void addWidget(QWidget *w, int stretch=0, Qt::Alignment align = Qt::Alignment());
};


#endif // QHBOXFRAME
