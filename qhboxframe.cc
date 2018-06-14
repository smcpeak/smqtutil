// qhboxframe.cc
// code for qhboxframe.h; tests are in test-layout.cc

#include "qhboxframe.h"                // this module

QHBoxFrame::QHBoxFrame(QWidget *parent, Qt::WindowFlags f)
  : QFrame(parent, f),
    hbox(new QHBoxLayout(this))
{
  this->hbox->setSpacing(0);
  this->hbox->setContentsMargins(0, 0, 0, 0);
}


QHBoxFrame::~QHBoxFrame()
{}


void QHBoxFrame::addWidget(QWidget *w, int stretch, Qt::Alignment align)
{
  this->hbox->addWidget(w, stretch, align);
}


// EOF
