// gdvalue-qt.cc
// Code for `gdvalue-qt` module.

#include "gdvalue-qt.h"                // this module

#include "smbase/gdvalue-parser.h"     // gdv::GDValueParser
#include "smbase/gdvalue-tuple.h"      // gdv::gdvpToTuple
#include "smbase/gdvalue.h"            // gdv::GDValue

#include "smqtutil/qtutil.h"           // toString(QString), toStringOpt(QEvent::Type)

#include <QMouseEvent>
#include <QPoint>
#include <QRect>

#include <climits>                     // CHAR_BIT
#include <optional>                    // std::optional
#include <tuple>                       // std::make_from_tuple

using namespace gdv;


gdv::GDValue toGDValue(QPoint const &p)
{
  return GDValue(GDVTaggedTuple("QPoint"_sym, {
    toGDValue(p.x()),
    toGDValue(p.y())
  }));
}


gdv::GDValue toGDValue(QPointF const &p)
{
  return GDValue(GDVTaggedTuple("QPointF"_sym, {
    toGDValue(p.x()),
    toGDValue(p.y())
  }));
}


gdv::GDValue toGDValue(QRect const &r)
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "QRect"_sym);
  m.mapSetValueAtSym("left", r.left());
  m.mapSetValueAtSym("top", r.top());
  m.mapSetValueAtSym("width", r.width());
  m.mapSetValueAtSym("height", r.height());
  return m;
}


gdv::GDValue toGDValue(QString const &str)
{
  return GDValue(toString(str));
}


gdv::GDValue toGDValue(QSize const &sz)
{
  return GDValue(GDVTaggedTuple("QSize"_sym, {
    toGDValue(sz.width()),
    toGDValue(sz.height())
  }));
}


namespace gdv {
  /*static*/ QSize GDVPTo<QSize>::f(GDValueParser const &p)
  {
    p.checkTaggedTupleSize("QSize", 2);
    return std::make_from_tuple<QSize>(
      gdvpToTuple<int, int>(p));
  }

  /*static*/ QPoint GDVPTo<QPoint>::f(GDValueParser const &p)
  {
    p.checkTaggedTupleSize("QPoint", 2);
    return std::make_from_tuple<QPoint>(
      gdvpToTuple<int, int>(p));
  }
}


GDValue toGDValue(QEvent::Type eventType)
{
  if (char const *name = toStringOpt(eventType)) {
    return GDVSymbol(name);
  }
  else {
    // Unknown event ID is treated as a user-defined ID.
    return GDVTaggedTuple("UserEvent"_sym, GDVTuple{int(eventType)});
  }
}


GDValue toGDValue(Qt::KeyboardModifier mod)
{
  if (char const *name = toStringOpt(mod)) {
    return GDVSymbol(name);
  }
  else {
    return GDVTaggedTuple("UnknownKeyboardModifier"_sym,
                          GDVTuple{int(mod)});
  }
}


GDValue toGDValue(Qt::MouseButton button)
{
  if (char const *name = toStringOpt(button)) {
    return GDVSymbol(name);
  }
  else {
    return GDVTaggedTuple("UnknownMouseButton"_sym,
                          GDVTuple{int(button)});
  }
}


template <typename FLAG>
GDValue flagsToGDValue(QFlags<FLAG> flags)
{
  GDValue s(GDVK_SET);

  // Check each candidate flag using a naive loop.
  for (int bitIndex = 0;
       bitIndex < int((sizeof(unsigned) * CHAR_BIT));
       ++bitIndex) {
    unsigned flag = (1u << bitIndex);
    if (flags & flag) {
      s.setInsert(toGDValue(FLAG(flag)));
    }
  }

  return s;
}


gdv::GDValue toGDValue(Qt::KeyboardModifiers mods)
{
  return flagsToGDValue(mods);
}


gdv::GDValue toGDValue(Qt::MouseButtons buttons)
{
  return flagsToGDValue(buttons);
}


namespace {
  // Parse `p` into a `T`, which is described as a `category`, and which
  // serializes to a tuple tagged with `unrecognized` when it is.
  template <typename T>
  T gdvpToEnumerator(
    GDValueParser const &p,
    char const *category,
    char const *unrecognized)
  {
    if (p.isSymbol()) {
      if (std::optional<T> eventTypeOpt =
            qtEnumeratorFromNameOpt<T>(p.symbolGetName())) {
        return *eventTypeOpt;
      }
      else {
        p.throwUnrecognizedSymbol(category);
      }
    }

    p.checkTaggedTupleSize(unrecognized, 1);
    return T(p.tupleGetValueAt(0).integerGetAs<int>());
  }

  // Parse `p` into a `QFlags`.
  template <typename T>
  QFlags<T> gdvpToQFlags(
    GDValueParser const &p)
  {
    QFlags<T> ret;

    for (GDValue const &elt : p.setGet()) {
      // TODO: Provide a way to directly iterate over parsers instead of
      // using this two-step procedure.
      GDValueParser eltParser = p.setGetValue(elt);

      ret |= gdvpTo<T>(eltParser);
    }

    return ret;
  }
}


namespace gdv {
  /*static*/ QEvent::Type GDVPTo<QEvent::Type>::f(
    GDValueParser const &p)
  {
    return gdvpToEnumerator<QEvent::Type>(p,
      "QEvent::Type", "UserEvent");
  }

  /*static*/ Qt::KeyboardModifier GDVPTo<Qt::KeyboardModifier>::f(
    GDValueParser const &p)
  {
    return gdvpToEnumerator<Qt::KeyboardModifier>(p,
      "Qt::KeyboardModifier", "UnknownKeyboardModifier");
  }

  /*static*/ Qt::KeyboardModifiers GDVPTo<Qt::KeyboardModifiers>::f(
    GDValueParser const &p)
  {
    return gdvpToQFlags<Qt::KeyboardModifier>(p);
  }

  /*static*/ Qt::MouseButton GDVPTo<Qt::MouseButton>::f(
    GDValueParser const &p)
  {
    return gdvpToEnumerator<Qt::MouseButton>(p,
      "Qt::MouseButton", "UnknownMouseButton");
  }

  /*static*/ Qt::MouseButtons GDVPTo<Qt::MouseButtons>::f(
    GDValueParser const &p)
  {
    return gdvpToQFlags<Qt::MouseButton>(p);
  }
}


#define SET_EVENT_FIELD(field) \
  m.mapSetValueAtSym(#field, toGDValue(ev.field())) /* user ; */


// This function is not part of the public module interface because it
// would be misleading to use it alone.  `QEvent` is abstract, so the
// return value would always be an incomplete description.
static GDValue toGDValue(QEvent const &ev)
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "QEvent"_sym);

  SET_EVENT_FIELD(type);
  SET_EVENT_FIELD(spontaneous);
  SET_EVENT_FIELD(isAccepted);

  return m;
}


// Not public since `QInputEvent` is abstract.
static GDValue toGDValue(QInputEvent const &ev)
{
  GDValue m = toGDValue(static_cast<QEvent const &>(ev));
  m.taggedContainerSetTag("QInputEvent"_sym);

  SET_EVENT_FIELD(modifiers);
  SET_EVENT_FIELD(timestamp);

  return m;
}


GDValue toGDValue(QMouseEvent const &ev)
{
  GDValue m = toGDValue(static_cast<QInputEvent const &>(ev));
  m.taggedContainerSetTag("QMouseEvent"_sym);

  SET_EVENT_FIELD(button);
  SET_EVENT_FIELD(buttons);

  // I expect to primarily interact with the integer-valued coordinates,
  // so that is what I will serialize, even though what is stored
  // internally is a `double`.
  SET_EVENT_FIELD(pos);

  // For the window position, only the floating-point form is exposed.
  SET_EVENT_FIELD(windowPos);

  // This one is an integer coordinate again.
  SET_EVENT_FIELD(globalPos);

  return m;
}


#undef SET_EVENT_FIELD


// EOF
