// col-width-rules.cc
// Code for `col-width-rules` module.

#include "smbase/gdvalue-optional-fwd.h"         // gdv::GDValue(std::optional)
#include "smbase/gdvalue-vector-fwd.h"           // gdv::GDValue(std::vector)

#include "col-width-rules.h"                     // this module

#include "smbase/gdvalue-optional.h"             // gdv::GDValue(std::optional)
#include "smbase/gdvalue-vector.h"               // gdv::GDValue(std::vector)
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/overflow.h"                     // safeToInt
#include "smbase/sm-macros.h"                    // IMEMBFP, IMEMBMFP
#include "smbase/sm-span-ops.h"                  // smbase::Span
#include "smbase/sm-span-util-ops.h"             // smbase::Span
#include "smbase/sm-trace.h"                     // INIT_TRACE, etc.
#include "smbase/vector-util.h"                  // vecSum
#include "smbase/xassert.h"                      // xassert, xassertPrecondition

#include <algorithm>                             // std::{min, max}
#include <cstdlib>                               // std::abs
#include <map>                                   // std::map
#include <optional>                              // std::optional
#include <utility>                               // std::move (h)
#include <vector>                                // std::vector

using namespace gdv;
using namespace smbase;


INIT_TRACE("col-width-rules");



// ------------------------------ ColSpec ------------------------------
ColumnWidthRules::ColSpec::ColSpec(
  int minimumSize,
  std::optional<int> maximumSize,
  int expansionPriority)
  : IMEMBFP(minimumSize),
    IMEMBFP(maximumSize),
    IMEMBFP(expansionPriority)
{
  selfCheck();
}


void ColumnWidthRules::ColSpec::selfCheck() const
{
  xassert(m_minimumSize >= 0);
  if (m_maximumSize) {
    xassert(*m_maximumSize >= m_minimumSize);
  }
}


ColumnWidthRules::ColSpec::operator gdv::GDValue() const
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "ColSpec"_sym);
  GDV_WRITE_MEMBER_SYM(m_minimumSize);
  GDV_WRITE_MEMBER_SYM(m_maximumSize);
  GDV_WRITE_MEMBER_SYM(m_expansionPriority);
  return m;
}


int ColumnWidthRules::ColSpec::flexibility(
  int curSize, bool expand, int maxFlex) const
{
  if (expand) {
    if (m_maximumSize) {
      if (curSize > *m_maximumSize) {
        return 0;
      }
      else {
        return *m_maximumSize - curSize;
      }
    }

    else {
      return maxFlex;
    }
  }
  else {
    if (curSize < m_minimumSize) {
      return 0;
    }
    else {
      return curSize - m_minimumSize;
    }
  }
}


bool ColumnWidthRules::ColSpec::clampSize(int &size /*INOUT*/) const
{
  int const origSize = size;

  size = std::max(size, m_minimumSize);

  if (m_maximumSize) {
    size = std::min(size, *m_maximumSize);
  }

  return size != origSize;
}


// ------------------------- ColumnWidthRules --------------------------
ColumnWidthRules::~ColumnWidthRules()
{}


ColumnWidthRules::ColumnWidthRules()
  : m_colSpecs(),
    m_nextColumnForUnevenDistribution(0)
{
  selfCheck();
}


ColumnWidthRules::ColumnWidthRules(std::vector<ColSpec> &&colSpecs)
  : IMEMBMFP(colSpecs),
    m_nextColumnForUnevenDistribution(0)
{
  selfCheck();
}


void ColumnWidthRules::selfCheck() const
{
  for (ColSpec const &cs : m_colSpecs) {
    cs.selfCheck();
  }
  xassert(m_nextColumnForUnevenDistribution >= 0);
}


ColumnWidthRules::operator gdv::GDValue() const
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "ColumnWidthRules"_sym);
  GDV_WRITE_MEMBER_SYM(m_colSpecs);
  return m;
}


void ColumnWidthRules::clear()
{
  m_colSpecs.clear();
  m_nextColumnForUnevenDistribution = 0;
}


int ColumnWidthRules::numColumns() const
{
  return safeToInt(m_colSpecs.size());
}


bool ColumnWidthRules::clampColumnSize(
  int columnIndex, int &size /*INOUT*/) const
{
  return m_colSpecs.at(columnIndex).clampSize(size);
}


// Increase or decrease (per `expand`) `value` by `delta`.
static void applyFlex(int &value /*INOUT*/, int delta, bool expand)
{
  if (expand) {
    value += delta;
  }
  else {
    value -= delta;
  }
}


bool ColumnWidthRules::resizeAll(
  Span<int> sizes, int newTotalSize)
{
  return resizeSome(0, sizes, newTotalSize);
}


bool ColumnWidthRules::resizeSome(
  int startColumnIndex,
  Span<int> changeableSizes,
  int newTotalSize)
{
  xassertPrecondition(
    startColumnIndex + changeableSizes.size() == m_colSpecs.size());

  // Number of columns that are in scope for resizing.
  int const numColumns = safeToInt(changeableSizes.size());

  // We need to add this many pixels total to the column widths in order
  // to match the viewport width.
  int const numPixelsToAdd = newTotalSize - spanSum(changeableSizes);
  if (numPixelsToAdd == 0) {
    return false;
  }

  // True to expand, false to contract.
  bool expand = numPixelsToAdd > 0;

  // Total number of pixels to expand or contract; always positive.
  int const totalNeededFlex = std::abs(numPixelsToAdd);

  // A vector of zeroes to serve as the initial value for the
  // following map.
  std::vector<int> const zeroes(numColumns, 0);

  // Map from priority value to sequence of column flexibilities,
  // which are nonzero only for columns that have that priority.
  std::map<int, std::vector<int>> prioToFlex;

  // For each column from among those in scope, calculate how much it
  // could adjust in the desired direction.
  {
    // This is an absolute index, i.e., it indexes `m_colSpecs`.
    int absColumnIndex = 0;

    for (ColSpec const &cs : m_colSpecs) {
      // Index relative to `startColumnIndex`, hence it indexes
      // `changeableSizes`.
      int relColumnIndex = absColumnIndex - startColumnIndex;

      if (relColumnIndex >= 0) {
        // Get the vector for this priority, creating it first if needed.
        auto it = prioToFlex.try_emplace(cs.m_expansionPriority, zeroes).first;

        // Compute the column flexibility and store that in the vector
        // for its priority.  The vector is indexed by a relative index.
        (*it).second[relColumnIndex] =
          cs.flexibility(
            changeableSizes.at(relColumnIndex), expand, totalNeededFlex);
      }

      ++absColumnIndex;
    }
  }

  // True if we changed anything.
  bool changed = false;

  // Vector that `evenlyDistribute` will populate.  It is hoisted here
  // to avoid unnecessary allocation inside the following loop.
  std::vector<int> flexToApply(numColumns);

  // Flex that we still need to apply.
  int remainingFlex = totalNeededFlex;

  // Process priority levels from highest to lowest.
  for (auto it = prioToFlex.crbegin();
       remainingFlex > 0 && it != prioToFlex.crend();
       ++it) {
    std::vector<int> const &flexes = (*it).second;

    // Distribute `remainingFlex` across the columns with non-zero
    // `flexes`.
    flexToApply.assign(numColumns, 0);
    evenlyDistribute(flexToApply /*INOUT*/, flexes, remainingFlex,
      m_nextColumnForUnevenDistribution /*INOUT*/);

    // Use `flexToApply` to update `changeableSizes`, etc.
    for (int i=0; i < numColumns; ++i) {
      if (int colFlex = flexToApply.at(i)) {
        applyFlex(changeableSizes.at(i) /*INOUT*/, colFlex, expand);
        remainingFlex -= colFlex;
        changed = true;
      }
    }
  }

  return changed;
}


bool ColumnWidthRules::resizeOne(
  int focusColumn,
  Span<int> sizes /*INOUT*/,
  int desiredTotalSize)
{
  xassertPrecondition(sizes.size() == m_colSpecs.size());
  xassertPrecondition(0 <= focusColumn &&
                           focusColumn < safeToInt(sizes.size()));

  bool changed = false;

  // Confine `focusColumn`.
  changed |=
    clampColumnSize(focusColumn, sizes.at(focusColumn) /*INOUT*/);

  // Attempt to satisfy the constraints by only changing the columns
  // after `focusColumn`.  This corresponds to a user grabbing the
  // divider between `focusColumn` and `focusColumn+1`, in which the
  // usual behavior is for the later columns to adjust as necessary.
  Span<int> leftSizes = sizes.subspan(0, focusColumn+1);
  Span<int> rightSizes = sizes.subspan(focusColumn+1);
  changed |=
    resizeSome(focusColumn+1, rightSizes,
      desiredTotalSize - spanSum(leftSizes));

  if (desiredTotalSize == spanSum(sizes)) {
    return changed;
  }

  // Try again, this time including `focusColumn`.  This has the effect
  // of restricting the user's ability to resize `focusColumn` when
  // doing so would cause the rightmost column to separate from the
  // viewport's right edge.
  leftSizes = sizes.subspan(0, focusColumn);
  rightSizes = sizes.subspan(focusColumn);
  changed |=
    resizeSome(focusColumn, rightSizes,
      desiredTotalSize - spanSum(leftSizes));

  if (desiredTotalSize == spanSum(sizes)) {
    return changed;
  }

  // Try once more with all columns in scope.  Normally we shouldn't get
  // here, but if resizing a column to the left will work, it's probably
  // best to do it now rather than wait for the entire window to be
  // resized.
  changed |=
    resizeAll(sizes, desiredTotalSize);

  return changed;
}


// ChatGPT assisted in writing this function implementation.
void evenlyDistribute(
  Span<int> const dest,
  Span<int const> const maxima,
  int const totalToDistribute,
  int &nextColumnForUnevenDistribution /*INOUT*/)
{
  xassertPrecondition(dest.size() == maxima.size());
  xassertPrecondition(totalToDistribute >= 0);

  // Name this for clarity.
  int const numElements = safeToInt(dest.size());

  // This will be decreased as we progress.
  int remaining = totalToDistribute;

  // Number of elements that can still take more, i.e., the number of
  // indices `i` for which `dest[i] < maxima[i]`.
  int numActive = 0;
  for (int m : maxima) {
    xassertPrecondition(m >= 0);
    if (m > 0) {
      ++numActive;
    }
  }

  int numIters = 0;
  while (remaining > 0 && numActive > 0) {
    // Each iteration should put at least one new element to its limit
    // if it doesn't finish the job entirely.  This check is primarily
    // here to guard against an infinite loop.
    xassert(numIters++ <= numElements);

    // If we ran the naive loop in the specification, what is the
    // minimum number of iterations it could run for?  We will try to
    // add this many to each active column.  This calculation assumes no
    // element hits its maximum; if one does, then we'll need at least
    // one more iteration of the outer loop.
    int tryToAdd = remaining / numActive;

    for (int i=0; i < numElements && remaining > 0; ++i) {
      // How much can it take?
      int const availableSpace = maxima[i] - dest[i];
      xassert(availableSpace >= 0);
      if (availableSpace == 0) {
        continue;
      }

      // Give it up to `tryToAdd`.
      int const toAdd = std::min(availableSpace, tryToAdd);

      dest[i] += toAdd;
      remaining -= toAdd;

      // Possibly stop adding to element `i`.
      if (dest[i] == maxima[i]) {
        --numActive;
      }
    }

    if (0 < remaining && remaining < numActive) {
      // We still have some pixels to distribute, but we cannot do so
      // evenly among the active elements.  Distribute them starting
      // with the persistent `nextColumnForUnevenDistribution`, as
      // explained in comments above this function's declaration.
      xassert(nextColumnForUnevenDistribution >= 0);

      for (int j=0; j < numElements && remaining > 0; ++j) {
        // Use this index instead of the naive `j`.
        int i = nextColumnForUnevenDistribution % numElements;

        if (dest[i] < maxima[i]) {
          dest[i] += 1;
          remaining -= 1;

          if (dest[i] == maxima[i]) {
            --numActive;
          }
        }

        // It's inelegant to do two mod operations, but I want the
        // process to start with index 0 (so I need a mod after the
        // increment), and I want to clamp the incoming `nextColumn`
        // since the number of columns could have changed, so I need one
        // before the increment too.
        nextColumnForUnevenDistribution = (i+1) % numElements;
      }

      xassert(remaining == 0);
    }
  }
}


// EOF
