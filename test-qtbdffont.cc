// test-qtbdffont.cc
// Tests for qtbdffont module.

#include "qtbdffont.h"                 // module to test

// this directory
#include "editor14r.bdf.gen.h"         // bdfFontData_editor14r
#include "lurs12.bdf.gen.h"            // bdfFontData_lurs12
#include "minihex6.bdf.gen.h"          // bdfFontData_minihex6
#include "qtutil.h"                    // toString(QRect)

// smbase
#include "bdffont.h"                   // BDFFont
#include "bit2d.h"                     // Bit2d::Size
#include "exc.h"                       // xbase
#include "strtokp.h"                   // StrtokParse
#include "test.h"                      // DEBUG_PVAL, ARGS_MAIN

// Qt
#include <qapplication.h>              // QApplication
#include <qimage.h>                    // QImage
#include <qlabel.h>                    // QLabel
#include <qpainter.h>                  // QPainter


ARGS_MAIN


// Test whether 'qfont' has the same information as 'font'.  Throw
// an exception if not.
static void compare(BDFFont const &font, QtBDFFont &qfont)
{
  xassert(font.maxValidGlyph() == qfont.maxValidChar());

  int glyphCount = 0;

  // Iterate over all potentially valid indices.
  for (int charIndex=0; charIndex <= font.maxValidGlyph(); charIndex++) {
    #define CHECK_EQUAL(a,b)                                       \
      if ((a) == (b)) {                                            \
        /* fine */                                                 \
      }                                                            \
      else {                                                       \
        xfailure(stringb("expected '" #a "' (" << (a) <<           \
                         ") to equal '" #b "' (" << (b) << ")"));  \
      }

    try {
      // Check for consistent presence in both.
      BDFFont::Glyph const *fontGlyph = font.getGlyph(charIndex);
      CHECK_EQUAL(fontGlyph!=NULL, qfont.hasChar(charIndex));
      if (fontGlyph == NULL) {
        continue;
      }
      glyphCount++;

      // Bounding box.
      QRect bbox = qfont.getCharBBox(charIndex);
      {
        // Compare to bbox from 'font'.  This basically repeats logic
        // from the QtBDFFont constructor; oh well.
        CHECK_EQUAL(bbox.width(), fontGlyph->metrics.bbSize.x);
        CHECK_EQUAL(bbox.height(), fontGlyph->metrics.bbSize.y);

        CHECK_EQUAL(bbox.left(), fontGlyph->metrics.bbOffset.x);
        
        // This formula is complicated because the meaning of
        // increasing 'y' values are reversed, and that in turn means
        // that the "top" point is a different corner than the
        // "offset" corner.
        //
        // Representative example for lowercase 'j', where X is
        // a drawn (black) pixel and O is the origin point.
        //
        // bbox coords
        // -----------
        //     -4       X   ^
        //     -3           |
        //     -2       X   |
        //     -1       X   |height=7
        //      0    O  X   |
        //      1    X  X   |
        //      2     XX    V
        //
        // fontGlyph->metrics.bbSize.y and bbox.height() are 7.
        //
        // fontGlyph->metrics.bbOffset.y is -2 since the bbox bottom
        // is 2 pixels below the origin point.
        //
        // bbox.top() is -4, which is (-7) + 1 + (-(-2)).
        //
        CHECK_EQUAL(bbox.top(), (-fontGlyph->metrics.bbSize.y) + 1 +
                                (-fontGlyph->metrics.bbOffset.y));
      }
      
      // Offset.
      QPoint offset = qfont.getCharOffset(charIndex);
      {
        // Compare to 'font'.
        point dWidth = fontGlyph->metrics.hasDWidth()?
                         fontGlyph->metrics.dWidth :
                         font.metrics.dWidth;
                         
        CHECK_EQUAL(offset.x(), dWidth.x);
        CHECK_EQUAL(offset.y(), -dWidth.y);
      }
      
      // Now the interesting part: make a temporary pixmap, render
      // the glyph on to it, then compare it to what is in 'font'.
              
      // The temporary pixmap will have 10 pixels of margin around the
      // sides so that we can detect if the renderer is drawing pixels
      // outside the claimed bounding box.
      enum { MARGIN = 10 };
      QPixmap pixmap(bbox.width() + MARGIN*2, bbox.height() + MARGIN*2);
      pixmap.fill(Qt::white);

      // Calculate the location of the origin pixel for the glyph if I
      // want the top-left corner of the bbox to go at (10,10).
      QPoint origin = QPoint(MARGIN,MARGIN) - bbox.topLeft();

      // Render the glyph.
      QPainter painter(&pixmap);
      qfont.drawChar(painter, origin, charIndex);

      // Now, convert the QPixmap into a QImage to allow fast access
      // to individual pixels.
      QImage image = pixmap.toImage();

      // Examine every pixel in 'image'.
      for (int y=0; y < image.height(); y++) {
        for (int x=0; x < image.width(); x++) {
          try {
            // Is the pixel black or white?
            bool isBlack;
            {
              QRgb rgb = image.pixel(x,y);
                   
              // first, make sure 'rgb' is either black or white;
              // it is up to the caller to ensure this
              xassert(qBlue(rgb) == qRed(rgb));
              xassert(qBlue(rgb) == qGreen(rgb));
              xassert(qBlue(rgb) == 0 || qBlue(rgb) == 255);

              isBlack = (qBlue(rgb) == 0);
            }

            // Margin area?
            if (x < MARGIN ||
                y < MARGIN ||
                x >= image.width() - MARGIN ||
                y >= image.height() - MARGIN) {
              CHECK_EQUAL(isBlack, false);
              continue;
            }

            // Which pixel does this correspond to in
            // 'fontGlyph->bitmap'?
            point corresp(x - MARGIN, y - MARGIN);
            CHECK_EQUAL(isBlack, fontGlyph->bitmap->get(corresp));
          }

          catch (xBase &exn) {
            exn.prependContext(stringb("pixel (" << x << ", " << y << ")"));
            throw;
          }
        } // loop over 'x'
      } // loop over 'y'
    }

    catch (xBase &x) {
      x.prependContext(stringb("index " << charIndex));
      throw;
    }

    #undef CHECK_EQUAL
  } // loop over 'charIndex'
  
  cout << "successfully compared " << glyphCount << " glyphs\n";
}


void entry(int argc, char **argv)
{
  BDFFont font;
  parseBDFString(font, bdfFontData_editor14r);

  QApplication app(argc, argv);

  // first compare with transparent drawing
  QtBDFFont qfont(font);
  compare(font, qfont);
  
  // then with opaque drawing
  qfont.setTransparent(false);
  compare(font, qfont);

  // Test another cycle of switching modes.
  qfont.setTransparent(true);
  compare(font, qfont);
  qfont.setTransparent(false);
  compare(font, qfont);

  cout << "test-qtbdffont console tests passed\n";
  if (argc >= 2 && 0==strcmp(argv[1], "gui")) {
    cout << "Running gui tests..." << endl;
  }
  else {
    cout << "Run with \"gui\" argument to test rendering to a window.\n";
    return;
  }

  // use different colors for onscreen drawing
  QColor fg = Qt::black;
  QColor bg(192, 192, 192);
  qfont.setFgColor(fg);
  qfont.setBgColor(bg);

  QLabel widget(NULL /*parent*/);
  widget.resize(600,300);

  QPixmap pixmap(600,300);
  pixmap.fill(bg);

  QPainter painter(&pixmap);
  painter.setPen(fg);
  painter.drawText(50,20, "QPainter::drawText");

  drawString(qfont, painter, QPoint(50,50),
             "drawString(QtBDFFont &)");

  qfont.setTransparent(true);
  drawString(qfont, painter, QPoint(50, 70),
             "setTransparent(true)");

  qfont.setTransparent(false);
  drawString(qfont, painter, QPoint(50, 90),
             "setTransparent(false)");

  // test rendering performance
  if (1) {
    // first transparent, then opaque
    for (int drawMode=0; drawMode < 2; drawMode++) {
      qfont.setTransparent(drawMode==0? true : false);

      long start = getMilliseconds();
      int iters = 10000;
      for (int i=0; i < iters; i++) {
        drawString(qfont, painter, QPoint(50,50),
                   "drawString(QtBDFFont &)");
      }
      long elapsed = getMilliseconds() - start;
      
      cout << (drawMode==0? "transparent: " : "opaque: ")
           << iters << " iters in " << elapsed << " ms" << endl;
    }
  }

  BDFFont font2;
  parseBDFString(font2, bdfFontData_lurs12);
  QtBDFFont qfont2(font2);
  qfont2.setFgColor(fg);
  qfont2.setBgColor(bg);
  qfont2.setTransparent(false);

  drawCenteredString(qfont2, painter, QPoint(150,110),
                     "some centered text blah blah blah blah blah");

  painter.drawLine(QPoint(0,150), QPoint(300,150));
  drawMultilineString(qfont2, painter, QPoint(0, 150),
                      "Entity iiiimmmm\n"
                      "RelationEndpoint\n"
                      "Diagram\n"
                      "ControlPoint\n"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ /0123456789\n"
                      "abcdefghijklmnopqrstuvwxyz\n");

  painter.drawLine(QPoint(300,0), QPoint(300,300));
  BDFFont minihexFont;
  parseBDFString(minihexFont, bdfFontData_minihex6);
  QtBDFFont minihexQFont(minihexFont);
  minihexQFont.setFgColor(fg);
  minihexQFont.setBgColor(bg);

  drawString(minihexQFont, painter, QPoint(320,20),
             "0123456789ABCDEF 0");

  {
    int const codePoints[] = {
      'h', 'e', 'l', 'l', 'o', ' ',
      0x0123, 0x4567, 0x89AB, 0xCDEF,
      0, 1, 2, 3,
    };

    QPoint pt(320, 40);
    for (int i=0; i < TABLESIZE(codePoints); i++) {
      pt = drawCharOrHexQuad(qfont, minihexQFont,
                             painter, pt, codePoints[i]);
    }
  }

  widget.setPixmap(pixmap);

  widget.show();

  app.exec();
}

// EOF
