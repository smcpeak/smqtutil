// qtbdffont.h
// module for writing to QPainters using BDF fonts
//
// This module relies in the smbase/bdffont.h module to obtain font
// glyphs.  It then copies that font information into a format
// suitable for efficiently drawing with it with Qt.
//
// It plays a role similar to QFont and QPainter::drawText, except
// that it does not rely on the underlying window system for any font
// or text rendering services.

// With regard to individual character indices, this module takes the
// same approach as the 'bdffont' module upon which it is built:
// characters are named using 'int' and no assumptions are made about
// the meaning of characters.
//
// However, the routines such as 'drawString' that accept a 'string'
// treat each byte as a character index, and thus are limited to
// character encoding systems with 256 characters or less.

// Performance considerations:
//
// Drawing with transparency is about 5-10x slower than drawing with
// opaque backgrounds (on X11; I have not tested on MS-Windows).
//
// Aside: The reason for this appears to be an unfortunate historical
// choice: the X-Window XCopyArea function unfortunately associates
// with mask origin with the *destination* rather than the source, so
// every call requires updating the origin, which invalidates the
// graphics context cache in the X server.  See
// http://tronche.com/gui/x/xlib/GC/manipulating.html .
//
// So, this module provides two modes, transparent backgrounds and
// opaque backgrounds, controlled by the 'transparent' attribute of
// QtBDFFont.
//
// Transparent backgrounds are slower, but far more flexible from a
// client's perspective.  The speed tends to be adequate for things
// like drawing programs with sparse amounts of text.
//
// Opaque backgrounds are faster to draw (e.g., fast enough for a text
// editor with lots of text to display), but require a single masked
// blit (5x slower again) in advance to create the source pixmap, so
// that speed is only realized when the foreground and background
// colors remain the same for many drawing operations.  It is up to
// the client to ensure that fg/bg is changed infrequently enough.
// For example, the client could make several QtBDFFont objects, one
// for each common fg/bg pair, and then one more for arbitrary fg/bg.
//
// Also note, as stated below, that the opaque background covers only
// the character's glyph bounding box, which is often much smaller
// than the "character cell" (if such a concept even makes sense).  So
// another burden transferred to the client is to manually erase the
// complete background rectangle before drawing the text.

#ifndef QTBDFFONT_H
#define QTBDFFONT_H

#include "array.h"                     // GrowArray
#include "str.h"                       // rostring

#include <qbitmap.h>                   // QBitmap, QPixmap
#include <qcolor.h>                    // QColor
#include <qpoint.h>                    // QPoint
#include <qrect.h>                     // QRect

#include <Qt>                          // Qt::Alignment

class BDFFont;                         // smbase/bdffont.h
class QPainter;                        // qpainter.h


// Store a font in a form suitable for drawing.
//
// In this class, X values increase going right, Y values increase
// going down.
//
// Some methods are marked 'const', but (for now) there is no useful
// notion of constness for this class, since it is semantically
// immutable.
class QtBDFFont {
  NO_OBJECT_COPIES(QtBDFFont);

private:     // types
  // Metrics about a single glyph.  Missing glyphs have all values set
  // to 0.
  class Metrics {
  public:    // data
    // Glyph bounding box in 'glyphMask' bitmap.
    QRect bbox;

    // Location of the glyph origin point in 'glyphMask'.  Not
    // necessarily inside 'bbox', nor even inside the dimensions of
    // 'glyphMask'.
    QPoint origin;

    // Relative amount by which to move the drawing point after
    // drawing this glyph.
    QPoint offset;

  public:
    Metrics();

    // Return true if this glyph is present, false if missing.
    bool isPresent() const;
  };

private:     // data
  // Bitmap containing all the font glyphs, packed together such that
  // no two overlap.  Other packing characteristics are implementation
  // details.
  QBitmap glyphMask;

  // A pixmap for use as the source in drawPixmap.  It has the same
  // size as 'glyphMask', and uses 'glyphMask' as its mask when
  // 'transparent' is true.
  //
  // It contains pixels of 'fgColor' and 'bgColor', with exact
  // arrangement dependent on 'colorPixmapState'.
  QPixmap colorPixmap;

  // Current foreground text color.  'colorPixmap' is filled with it.  This is
  // changed when an attempt is made to draw a different color.
  QColor fgColor;

  // Current background text color.
  QColor bgColor;

  // Current state of 'colorPixmap' relative to 'fgColor' and
  // 'bgColor'.
  enum ColorPixmapState {
    CPS_SOLID,               // entirely filled with 'fgColor'
    CPS_MIX                  // 'fgColor' foreground, 'bgColor' background
  };
  ColorPixmapState colorPixmapState;

  // Relative to the origin, the minimal bounding box that encloses
  // every glyph in the font.
  QRect allCharsBBox;

  // Map from character index to associated metrics.  It does not
  // "grow"; I use GrowArray for its bounds checking.
  GrowArray<Metrics> metrics;

  // Nominal font-wide metrics.  This is used, for example, to know
  // the proper size for a synthesized replacement glyph.
  Metrics nominalFontMetrics;

  // True if drawing operations will use transparent backgrounds,
  // false for opaque backgrounds.
  bool transparent;

private:     // funcs
  void createMixedColorPixmap();
  void createSolidColorPixmap();

public:      // funcs
  // This makes a copy of all required data in 'font'; 'font' can be
  // destroyed afterward.
  //
  // The initial drawing attributes black text on a white background,
  // but 'transparent' is true.
  QtBDFFont(BDFFont const &font);
  ~QtBDFFont();

  // Return the maximum valid character index, or -1 if there are no
  // valid indices.
  int maxValidChar() const;

  // Return true if there is a glyph with the given index.
  bool hasChar(int index) const;

  // Return the origin-relative bounding box of a glyph.  Will return
  // (0,0,0,0) if the glyph is missing.
  //
  // Note that any glyph with pixels above the origin point (which is
  // most of them) will have a negative value for the 'top' value of
  // the rectangle, because this class's coordinate system has Y
  // increasing going down.
  QRect getCharBBox(int index) const;

  // Return the origin-relative minimal bounding box for all glyphs.
  QRect const &getAllCharsBBox() const { return allCharsBBox; }

  // Return the offset by which the origin should move after drawing
  // a given glyph.  Returns (0,0) if the glyph is missing.
  QPoint getCharOffset(int index) const;

  // Using the nominal font-wide metrics, return a bbox for a character
  // cell when the basline point is 'pt'.
  QRect getNominalCharCell(QPoint pt) const;

  // Return the nominal vector from one glyph's baseline point to the
  // next.
  QPoint getNominalCharOffset() const;

  // Render a single character at 'pt'.
  //
  // If 'transparent' is true, only draw the foreground pixels using
  // the current foreground color.
  //
  // Otherwise, renders the entire *bounding box* opaquely, using the
  // current foreground and background colors.
  //
  // If there is no glyph with the given index, this is a no-op.
  void drawChar(QPainter &dest, QPoint pt, int index);

  // Get and set fg/bg colors.  Subsequent calls to 'drawChar'
  // will use these colors.
  QColor getFgColor() const { return fgColor; }
  QColor getBgColor() const { return bgColor; }
  void setFgColor(QColor const &newFgColor);
  void setBgColor(QColor const &newBgColor);

  // Set fg/bg to match another font.
  void setSameFgBgColors(QtBDFFont const &other);

  // Get and set 'transparent'.
  bool getTransparent() const { return transparent; }
  void setTransparent(bool newTransparent);
};


// Draw a string at 'pt'.
//
// The individual characters in 'str' are interpreted as 'unsigned
// char' for purposes of extracting a character index.  (See note at
// top of file.)
void drawString(QtBDFFont &font, QPainter &dest,
                QPoint pt, rostring str);


// For an entire string, calculate a bounding rectangle, assuming the
// origin is at (0,0).  As with 'getCharBBox', the top of the
// resulting rectangle will usually be negative.  Returns (0,0,0,0) if
// none of the glyphs in 'str' are present.
QRect getStringBBox(QtBDFFont &font, rostring str);


// Draw a string centered both horizontally and vertically about
// the given point, according to the glyph bbox metrics.
void drawCenteredString(QtBDFFont &font, QPainter &dest,
                        QPoint center, rostring str);


// Similar to QPainter::drawText(QRect, ...), draw the specified text in
// the given rectangle, aligned per 'alignment'.  Currently the only
// alignments supported are Left/Right/HCenter and Top/Bottom/VCenter.
// If a direction's alignment is not specified, it acts like "center".
//
// However, unlike QPainter::drawText, this function does *not* clip its
// output to 'rect'.
void drawAlignedString(QtBDFFont &font, QPainter &dest,
  QRect const &rect, Qt::Alignment alignment, string const &str);


// Draw a string that contains multiple newline-separated lines.
// The 'upLeft' is the upper-left corner to start at; it is *not*
// the starting origin.
void drawMultilineString(QtBDFFont &font, QPainter &dest,
                         QPoint upLeft, rostring str);


// Draw four uppercase hexadecimal characters in the given bounds
// rectangle arranged in a pattern like this:
//   +--+
//   |AB|
//   |CD|
//   +--+
// where 0xABCD is 'codePoint'.  The intended purpose is to draw a
// placeholder for a missing font glyph.  Currently this function
// does not clip its output to 'bounds', so an appropriately small
// font must be used.
void drawHexQuad(QtBDFFont &font, QPainter &paint,
                 QRect const &bounds, int codePoint);


// If 'codePoint' has a glyph in 'mainFont', draw it at 'pt'.
// Otherwise, use 'minihexFont' to draw a hex quad showing the code
// point value.  Either way, return the baseline point for the next
// glyph.
QPoint drawCharOrHexQuad(QtBDFFont &mainFont, QtBDFFont &minihexFont,
                         QPainter &dest, QPoint pt, int codePoint);


#endif // QTBDFFONT_H
