// qtbdffont.cc
// code for qtbdffont.h

#include "qtbdffont.h"                 // this module

#include "qtutil.h"                    // toString(QRect)

#include "bdffont.h"                   // BDFFont
#include "bit2d.h"                     // Bit2d::Size
#include "exc.h"                       // xbase
#include "strtokp.h"                   // StrtokParse
#include "test.h"                      // DEBUG_PVAL

#include <qimage.h>                    // QImage
#include <qpainter.h>                  // QPainter

#include <stdio.h>                     // snprintf (needs C99 or C++11)


// --------------------- QtBDFFont::Metrics -----------------------
QtBDFFont::Metrics::Metrics()
  : bbox(0,0,0,0),
    origin(0,0),
    offset(0,0)
{}


bool QtBDFFont::Metrics::isPresent() const
{
  // Must test offset as well as bbox because the glyph for ' ' can
  // have an empty bbox but still be present for its offset effect.
  if (bbox.width() == 0 && bbox.height() == 0 &&
      offset.x() == 0 && offset.y() == 0) {
    return false;
  }
  else {
    return true;
  }
}


// ------------------------- QtBDFFont --------------------------
// Compute the 'origin' of QtBDFFont::Metrics from GlyphMetrics.
static QPoint originFromGlyphMetrics(BDFFont::GlyphMetrics const &gmet)
{
  // If gmet.bbOffset had value (0,0), then the origin would be the
  // lower-left corner of the glyph bbox, which is (0, gmet.bbSize.y-1)
  // in the QtBDFFont coordinate system.
  //
  // From there, a positive offset of gmet.bbOffset.x is interpreted
  // as moving the bbox to the right, which is equivalent to moving
  // the origin left, so we subtract.
  //
  // A positive offset of gmet.bbOffset.y is interpreted as moving
  // the bbox *up* in the coordinate system of
  // BDFFont::GlyphMetrics, which is equivalent to moving the origin
  // down, so we add (since down is positive in the coordinate
  // system of 'glyphMask').
  return QPoint(- gmet.bbOffset.x,
                gmet.bbSize.y-1 + gmet.bbOffset.y);
}


QtBDFFont::QtBDFFont(BDFFont const &font)
  : glyphMask(),             // Null for now
    colorPixmap(),
    fgColor(0,0,0),          // black
    bgColor(255,255,255),    // white
    colorPixmapState(CPS_SOLID),
    allCharsBBox(0,0,0,0),
    metrics(font.glyphIndexLimit()),
    nominalFontMetrics(),
    transparent(true)
{
  // The main thing this constructor does is build the 'glyphMask'
  // bitmap and the 'metrics' array.  To do so, we pack the glyph
  // images into a rectangular bitmap.  In general, optimal packing is
  // NP-complete, and the benefit of efficiency here is not great, so
  // I use a completely naive strategy of putting them horizontally
  // adjacent, aligned at the bounding box top.
  //
  // However, it seems likely that with just a bit more work, I could
  // get a more efficient structure that tried to arrange glyphs in
  // several regular-height rows, with taller-than-usual glyphs off to
  // the side.  This would also have the advantage of reducing the
  // maximum dimension of the pixmap; the Qt docs explain that some
  // window systems have trouble with dimensions exceeding 4k.  But
  // I'll save those optimizations for another time.

  // maximum glyph height encountered so far
  int maxHeight = 0;

  // current horizontal position for the next glyph's left edge
  int currentX = 0;

  // Pass 1: Compute 'metrics'.
  for (int i=0; i < metrics.size(); i++) {
    BDFFont::Glyph const *glyph = font.getGlyph(i);
    if (!glyph) {
      // metrics[i] should already be zeroed
      continue;
    }
    BDFFont::GlyphMetrics const &gmet = glyph->metrics;

    // place glyph 'i' here
    metrics[i].bbox = QRect(currentX, 0,
                            gmet.bbSize.x, gmet.bbSize.y);
    metrics[i].origin = QPoint(currentX, 0) + originFromGlyphMetrics(gmet);

    // Get movement offset, which might come from 'font'.
    point dWidth = gmet.hasDWidth()?
                     gmet.dWidth :
                     font.metrics.dWidth;

    // Origin movement offset.  Same as 'dWidth', except again the 'y'
    // axis inverted.  Except, you'd never know, since in practice it
    // will always be 0.
    metrics[i].offset = QPoint(dWidth.x, -dWidth.y);

    // bump variables involved in packing calculation
    maxHeight = max(maxHeight, gmet.bbSize.y);
    currentX += gmet.bbSize.x;

    // Update 'allCharsBBox'.  This call reads from 'metrics[i]'.
    allCharsBBox |= getCharBBox(i);
  }

  // Grab font-wide metrics.
  {
    BDFFont::GlyphMetrics const &gmet = font.metrics;
    this->nominalFontMetrics.bbox =
      QRect(0, 0, gmet.bbSize.x, gmet.bbSize.y);
    this->nominalFontMetrics.origin = originFromGlyphMetrics(gmet);

    // It is a little sketchy to just assume that the bbox provides
    // a good inter-character offset, but I don't have any other
    // metric to use.
    this->nominalFontMetrics.offset = QPoint(gmet.bbSize.x, 0);
  }

  // Allocate an image with the same size as 'glyphMask' will
  // ultimately be.  I use a QImage here because I'm going to use the
  // slow method of copying individual pixels, for now, and a
  // QPixmap/QBitmap is very slow at accessing individual pixels.
  //
  // Using MonoLSB instead of Mono is a small optimization, since
  // internally QBitmap::fromImage will convert to MonoLSB.
  QImage tempMask(currentX,                      // width
                  maxHeight,                     // height
                  QImage::Format_MonoLSB);

  // Strangely, although QImage defaults to 0=black and 1=white,
  // QBitmap::fromImage expects the opposite, and will invert the
  // bits if we don't pre-set the colors.
  tempMask.setColor(0, QColor(Qt::color0).rgb());
  tempMask.setColor(1, QColor(Qt::color1).rgb());

  // Start with 0 (transparent).
  tempMask.fill(0);

  // Pass 2: Copy the glyph images using the positions calculated
  // above.
  for (int i=0; i < metrics.size(); i++) {
    BDFFont::Glyph const *glyph = font.getGlyph(i);
    if (!glyph) {
      continue;
    }

    if (!glyph->bitmap) {
      continue;      // nothing to copy
    }
    xassert(glyph->bitmap->Size() == glyph->metrics.bbSize);

    // Copy the pixels one by one.
    //
    // This could be made faster by doing low-level bit manipulation,
    // but the Qt docs are a little vague about exactly what would be
    // required, and this isn't a bottleneck anyway.
    for (int y=0; y < glyph->metrics.bbSize.y; y++) {
      for (int x=0; x < glyph->metrics.bbSize.x; x++) {
        if (glyph->bitmap->get(point(x,y))) {
          tempMask.setPixel(metrics[i].bbox.x() + x,
                            metrics[i].bbox.y() + y,
                            1);
        }
      }
    }
  }

  // Create 'glyphs' from 'tempGlyphs'.  This allocates, converts the
  // data from QImage to QBitmap, and copies it to the window system.
  glyphMask = QBitmap::fromImage(tempMask);

  // Create 'colorPixmap', initially just solid 'fgColor'.
  colorPixmap = QPixmap(glyphMask.size());
  colorPixmap.fill(fgColor);

  // Associate it as the mask due to 'transparent'.
  colorPixmap.setMask(glyphMask);
}


QtBDFFont::~QtBDFFont()
{}


int QtBDFFont::maxValidChar() const
{
  int ret = metrics.size() - 1;
  while (ret >= 0 && !hasChar(ret)) {
    ret--;
  }
  return ret;
}


bool QtBDFFont::hasChar(int index) const
{
  if (0 <= index && index < metrics.size()) {
    return metrics[index].isPresent();
  }
  else {
    return false;
  }
}


QRect QtBDFFont::getCharBBox(int index) const
{
  if (hasChar(index)) {
    QRect ret(metrics[index].bbox);
    ret.translate(- metrics[index].origin);
    return ret;
  }
  else {
    return QRect(0,0,0,0);
  }
}


QPoint QtBDFFont::getCharOffset(int index) const
{
  if (hasChar(index)) {
    return metrics[index].offset;
  }
  else {
    return QPoint(0,0);
  }
}


QRect QtBDFFont::getNominalCharCell(QPoint pt) const
{
  QRect ret(this->nominalFontMetrics.bbox);
  ret.translate(- this->nominalFontMetrics.origin);
  ret.translate(pt);
  return ret;
}


QPoint QtBDFFont::getNominalCharOffset() const
{
  return this->nominalFontMetrics.offset;
}


// Set 'colorPixmapState' to 'CPS_MIX', and modify 'colorPixmap'
// accordingly.
void QtBDFFont::createMixedColorPixmap()
{
  // This should only be called when 'transparent' is false, which
  // means that 'glyphMask' is note associated with any pixmaps right
  // now.
  xassert(transparent == false);

  // Fill the entire 'colorPixmap' with the background color.
  colorPixmap.fill(bgColor);

  // Create a new pixmap to act as the source for the masked blit.
  //
  // TODO: It's possible there is a way to do this without making a
  // temporary pixmap, but I'm not sure right now because it's not
  // clear exactly when a QPixmap's mask is used.
  QPixmap temp(colorPixmap.size());
  temp.fill(fgColor);

  // Do a masked blit of 'fgColor' onto 'colorPixmap'.
  temp.setMask(glyphMask);
  QPainter painter(&(this->colorPixmap));
  painter.drawPixmap(0,0, temp);

  // Reflect new state.
  colorPixmapState = CPS_MIX;
}


// Set 'colorPixmapState' to 'CPS_SOLID' and modify 'colorPixmap'
// accordingly.
void QtBDFFont::createSolidColorPixmap()
{
  colorPixmap.fill(fgColor);
  colorPixmapState = CPS_SOLID;
}


void QtBDFFont::drawChar(QPainter &dest, QPoint pt, int index)
{
  if (!hasChar(index)) {
    return;
  }
  Metrics const &met = metrics[index];

  if (met.bbox.isEmpty()) {
    // This has to be excluded as a special case because
    // QPainter::drawPixmap treats w=h=0 as meaning "draw
    // the entire source image".
    return;
  }

  // Make sure 'transparent' and 'colorPixmapState' agree.
  if (transparent==false && colorPixmapState!=CPS_MIX) {
    createMixedColorPixmap();
  }

  // Upper-left corner of rectangle to copy, in the 'dest' coords.
  pt -= (met.origin - met.bbox.topLeft());

  // Copy the image.
  dest.drawPixmap(pt,                  // upper-left of dest rectangle
                  colorPixmap,         // source pixmap
                  met.bbox);           // source rectangle
}


void QtBDFFont::setFgColor(QColor const &newFgColor)
{
  if (fgColor != newFgColor) {
    fgColor = newFgColor;
    createSolidColorPixmap();
  }
}


void QtBDFFont::setBgColor(QColor const &newBgColor)
{
  if (bgColor != newBgColor) {
    bgColor = newBgColor;
    if (colorPixmapState == CPS_MIX) {
      createSolidColorPixmap();
    }
  }
}


void QtBDFFont::setSameFgBgColors(QtBDFFont const &other)
{
  this->setFgColor(other.getFgColor());
  this->setBgColor(other.getBgColor());
}


void QtBDFFont::setTransparent(bool newTransparent)
{
  if (transparent != newTransparent) {
    transparent = newTransparent;

    if (transparent) {
      // The foreground pixels should already have the
      // foreground color, so mask out the bg pixels.
      colorPixmap.setMask(glyphMask);
    }
    else {
      // Prepare by setting the entire pixmap to the fg
      // color and the state to CPS_SOLID.  'drawChar'
      // will then be able to set things up properly.
      // (This is a weird design.  It arose due to
      // porting Qt3 code to Qt5.)
      this->createSolidColorPixmap();
    }
  }
}


// ------------------- global functions ----------------------
void drawString(QtBDFFont &font, QPainter &dest,
                QPoint pt, rostring str)
{
  for (char const *p = str.c_str(); *p; p++) {
    // Interpret each byte as a character index, unsigned
    // because no encoding system uses negative indices.
    int charIndex = (unsigned char)*p;

    font.drawChar(dest, pt, charIndex);
    pt += font.getCharOffset(charIndex);
  }
}


QRect getStringBBox(QtBDFFont &font, rostring str)
{
  // Loop to search for the first valid glyph.
  for (char const *p = str.c_str(); *p; p++) {
    int charIndex = (unsigned char)*p;

    if (font.hasChar(charIndex)) {
      // Begin actual bbox calculation.

      // Accumulated bbox.  Start with first character's bbox.
      QRect ret(font.getCharBBox(charIndex));

      // Virtual cursor; where to place next glyph.
      QPoint cursor = font.getCharOffset(charIndex);

      // Add the bboxes for subsequent characters.
      for (p++; *p; p++) {
        int charIndex = (unsigned char)*p;
        QRect glyphBBox = font.getCharBBox(charIndex);
        glyphBBox.translate(cursor.x(), cursor.y());
        ret |= glyphBBox;

        cursor += font.getCharOffset(charIndex);
      }

      return ret;
    }
  }

  // No valid glyphs.
  return QRect(0,0,0,0);
}


void drawCenteredString(QtBDFFont &font, QPainter &dest,
                        QPoint center, rostring str)
{
  drawAlignedString(font, dest, QRect(center, QSize(0,0)),
    (Qt::AlignHCenter | Qt::AlignVCenter), str);
}


void drawAlignedString(QtBDFFont &font, QPainter &dest,
  QRect const &rect, Qt::Alignment alignment, string const &str)
{
  // Calculate a bounding rectangle for the entire string if it were to
  // be drawn at (0,0).
  QRect bbox = getStringBBox(font, str);

  // Desired rectangle left edge.
  int left = (alignment & Qt::AlignLeft)?
               rect.left() :
             (alignment & Qt::AlignRight)?
               rect.left() + rect.width() - bbox.width() :
             /*else*/
               rect.left() + (rect.width() - bbox.width())/2;

  // Desired rectangle top edge.
  int top  = (alignment & Qt::AlignTop)?
               rect.top() :
             (alignment & Qt::AlignBottom)?
               rect.top() + rect.height() - bbox.height() :
             /*else*/
               rect.top() + (rect.height() - bbox.height())/2;

  // Package as a point.
  QPoint pt(left, top);

  // Shift to where the beginning of the baseline should go.
  pt -= bbox.topLeft();

  // Draw at that baseline point.
  drawString(font, dest, pt, str);
}


void drawMultilineString(QtBDFFont &font, QPainter &dest,
                         QPoint upLeft, rostring str)
{
  // adjust 'upLeft' so it is the starting origin
  upLeft += -font.getAllCharsBBox().topLeft();

  // split 'str' into lines
  StrtokParse tok(str, "\r\n");
  for (int i=0; i < tok.tokc(); i++) {
    drawString(font, dest, upLeft, tok[i]);

    upLeft.setY(upLeft.y() + font.getAllCharsBBox().height());
  }
}


void drawHexQuad(QtBDFFont &font, QPainter &paint,
                 QRect const &bounds, int codePoint)
{
  int midx = bounds.center().x();
  int top = bounds.top();
  int h = bounds.height();

  char buf[3];

  snprintf(buf, sizeof(buf), "%02X", ((codePoint >> 8) & 0xFF));
  drawCenteredString(font, paint, QPoint(midx, top + h/4), buf);

  snprintf(buf, sizeof(buf), "%02X", (codePoint & 0xFF));
  drawCenteredString(font, paint, QPoint(midx, top + h*3/4), buf);
}


QPoint drawCharOrHexQuad(QtBDFFont &mainFont, QtBDFFont &minihexFont,
                         QPainter &dest, QPoint pt, int codePoint)
{
  if (mainFont.hasChar(codePoint)) {
    mainFont.drawChar(dest, pt, codePoint);
    pt += mainFont.getCharOffset(codePoint);
  }
  else {
    QRect bounds = mainFont.getNominalCharCell(pt);
    drawHexQuad(minihexFont, dest, bounds, codePoint);
    pt += mainFont.getNominalCharOffset();
  }
  return pt;
}


// EOF
