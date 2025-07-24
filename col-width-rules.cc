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
#include "smbase/sm-trace.h"                     // INIT_TRACE, etc.
#include "smbase/vector-util.h"                  // vecSum
#include "smbase/xassert.h"                      // xassert, xassertPrecondition

#include <algorithm>                             // std::min
#include <cstdlib>                               // std::abs
#include <map>                                   // std::map
#include <optional>                              // std::optional
#include <utility>                               // std::move (h)
#include <vector>                                // std::vector

using namespace gdv;


INIT_TRACE("col-width-rules");



// ------------------------------ ColSpec ------------------------------
ColumnWidthRules::ColSpec::ColSpec()
  : m_minimumSize(0),
    m_maximumSize(),
    m_expansionPriority(1)
{
  selfCheck();
}


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


// ------------------------- ColumnWidthRules --------------------------
ColumnWidthRules::~ColumnWidthRules()
{}


ColumnWidthRules::ColumnWidthRules()
  : m_colSpecs()
{
  selfCheck();
}


ColumnWidthRules::ColumnWidthRules(std::vector<ColSpec> &&colSpecs)
  : IMEMBMFP(colSpecs)
{
  selfCheck();
}


void ColumnWidthRules::selfCheck() const
{
  for (ColSpec const &cs : m_colSpecs) {
    cs.selfCheck();
  }
}


ColumnWidthRules::operator gdv::GDValue() const
{
  GDValue m(GDVK_TAGGED_ORDERED_MAP, "ColumnWidthRules"_sym);
  GDV_WRITE_MEMBER_SYM(m_colSpecs);
  return m;
}


int ColumnWidthRules::numColumns() const
{
  return safeToInt(m_colSpecs.size());
}


// Increase or decrease (per `expand`) `value` by `delta`.
static void applyFlex(int /*INOUT*/ &value, int delta, bool expand)
{
  if (expand) {
    value += delta;
  }
  else {
    value -= delta;
  }
}


bool ColumnWidthRules::resizeAll(
  std::vector<int> /*INOUT*/ &sizes, int newTotalSize) const
{
  xassertPrecondition(sizes.size() == m_colSpecs.size());
  xassertPrecondition(newTotalSize >= 0);

  int const numColumns = safeToInt(m_colSpecs.size());

  // We need to add this many pixels total to the column widths in order
  // to match the viewport width.
  int const numPixelsToAdd = newTotalSize - vecSum(sizes);
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
  // which are nonzero only for columns that have that priorty.
  std::map<int, std::vector<int>> prioToFlex;

  // For each column, calculate how much it could adjust in the
  // desired direction.
  {
    int columnIndex = 0;
    for (ColSpec const &cs : m_colSpecs) {
      // Get the vector for this priority.
      auto it = prioToFlex.try_emplace(cs.m_expansionPriority, zeroes).first;

      // Compute the column flexibility and store that in the vector
      // for its priority.
      (*it).second[columnIndex] =
        cs.flexibility(sizes.at(columnIndex), expand, totalNeededFlex);

      ++columnIndex;
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
    evenlyDistribute(flexToApply /*INOUT*/, flexes, remainingFlex);

    // Use `flexToApply` to update `sizes`, etc.
    for (int i=0; i < numColumns; ++i) {
      if (int colFlex = flexToApply.at(i)) {
        applyFlex(sizes.at(i) /*INOUT*/, colFlex, expand);
        remainingFlex -= colFlex;
        changed = true;
      }
    }
  }

  return changed;
}


// ChatGPT assisted in writing this function implementation.
void evenlyDistribute(
  std::vector<int> /*INOUT*/ &dest,
  std::vector<int> const &maxima,
  int const totalToDistribute)
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
    // *minimum* number of iterations it could run for?  This
    // calculation assumes no element hits its maximum; if one does,
    // then we'll need at least one more iteration of the outer loop.
    int naiveIterations = remaining / numActive;
    if (naiveIterations == 0) {
      // We know we need at one since we're not done.
      naiveIterations = 1;
    }

    for (int i=0; i < numElements && remaining > 0; ++i) {
      // How much can it take?
      int const available = maxima[i] - dest[i];
      xassert(available >= 0);
      if (available == 0) {
        continue;
      }

      // Give it up to `naiveIterations`.
      int const toAdd = std::min(available, naiveIterations);

      dest[i] += toAdd;
      remaining -= toAdd;

      // Possibly stop adding to element `i`.
      if (dest[i] == maxima[i]) {
        --numActive;
      }
    }
  }
}


// EOF
