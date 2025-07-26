// col-width-rules.h
// `ColumnWidthRules` class.

// See license.txt for copyright and terms of use.

#ifndef SMQTUTIL_COL_WIDTH_RULES_H
#define SMQTUTIL_COL_WIDTH_RULES_H

#include "smbase/gdvalue-fwd.h"        // gdv::GDValue

#include <optional>                    // std::optional
#include <vector>                      // std::vector


// This class provides a system of constraints on a sequence of integer
// values, presumed to be the pixel widths of a set of resizable columns
// of a table widget, along with algorithms for solving the constraints.
class ColumnWidthRules {
public:      // types
  // Specification of one column's constraints.
  class ColSpec {
  public:      // data
    // Ensure the column is at least this wide.
    //
    // Invariant: Non-negative.
    int m_minimumSize;

    // If set, ensure the column is at most this wide.
    //
    // Invariant: If set, is at least `m_minimumSize`.
    std::optional<int> m_maximumSize;

    // Whenever a set of columns needs to expand or contract as a whole,
    // the columns with the highest priority are adjusted first.
    int m_expansionPriority;

  public:      // methods
    ColSpec(
      int minimumSize = 0,
      std::optional<int> maximumSize = std::nullopt,
      int expansionPriority = 1);

    // Check invariants.
    void selfCheck() const;

    operator gdv::GDValue() const;

    // If the column width is currently `curSize`, by how many pixels
    // could we expand or contract (per `expand`)?
    //
    // If `expand==true` and `m_maximumSize` is unset, return `maxFlex`.
    //
    // This tolerates a value of `curSize` that is already outside the
    // allowed range for this column, returning a 0 flexibility in that
    // case.
    int flexibility(int curSize, bool expand, int maxFlex) const;

    // Return the value closest to `size` that satisfies the size
    // constraints.
    int clampSize(int size) const;
  };

public:      // data
  // Current column specifications.
  std::vector<ColSpec> m_colSpecs;

  // Index to help with uneven distributions.  See comments above the
  // declaration of `evenlyDistribute`.
  //
  // Invariant: Non-negative.
  int m_nextColumnForUnevenDistribution;

public:      // methods
  ~ColumnWidthRules();

  // Empty sequence of columns.
  explicit ColumnWidthRules();

  explicit ColumnWidthRules(std::vector<ColSpec> &&colSpecs);

  // Check invariants.
  void selfCheck() const;

  operator gdv::GDValue() const;

  // Get the number of columns per `m_colSpecs`.
  int numColumns() const;

  // Return the value closest to `size` that is within the specified
  // bounds for the indicated column.
  int clampColumnSize(int columnIndex, int size) const;

  // Given `sizes`, the current column sizes, modify it so their total
  // size is `newTotalWidth` (which can be negative), or as close to
  // that as possible while respecting the column limits.
  //
  // The algorithm operates one priority level at a time, in descending
  // order, using `evenlyDistribute` (below) to either expand or
  // contract columns.  If the flexibility at a given priority level is
  // exhausted without achieving `newTotalSize`, it moves to the next
  // lower priority.
  //
  // This is not `const` due to how `m_nextColumnForUnevenDistribution`
  // is used.
  //
  // Return true iff at least one element of `sizes` was changed.
  //
  bool resizeAll(
    std::vector<int> /*INOUT*/ &sizes, int newTotalSize);

  // Same as `resizeAll`, but only operate on the columns starting with
  // `startColumn`.
  bool resizeSome(
    int startColumn,
    std::vector<int> /*INOUT*/ &sizes,
    int newTotalSize);
};


/*
  Given:

  * dest: A vector of initially all zeroes.

  * maxima: A vector of the same size containing non-negative
    values that specify the maximum value to write into the
    corresponding element of `dest`.

  * totalToDistribute: A non-negative integer specifying the desired
    total value of elements in `dest` at the end.

  This function efficiently populates `dest` as if by the following
  (inefficient) algorithm:

    set `remaining` to totalToDistribute

    while remaining > 0:
      for all i:
        if remaining > 0 and dest[i] < maxima[i]:
          increment dest[i] by one
          decrement remaining by one

  That is, we evenly distribute the `totalToDistribute` across the
  elements of `dest` up to their respective maxima.

  Note that the final iteration of the expository loop might only add
  one to some but not all of the elements that are less than their
  maximum due to `remaining` being less than the number of such
  elements.

  Furthermore, as a refinement, that final iteration should not actually
  add the final units starting with element 0, but rather starting with
  element `nextColumnForUnevenDistribution`, modulo `dest.size()`.  The
  `nextColumn` value will be incremented by the number of uneven units
  added, such that on subsequent calls, the units will be distributed to
  successive elements rather than all piling onto the first.  This is
  important when this function is used to react to window resize events,
  since the naive approach causes the first element (e.g, table column)
  to be the only one that changes if the resize is performed slowly.

  The vector `dest` is passed by reference, rather than returned,
  because this function is called repeatedly in a loop and we will reuse
  the vector object (and its storage) across iterations.

  This function is only intended to be used internally by this module,
  but exposed in the interface to allow direct unit testing.
*/
void evenlyDistribute(
  std::vector<int> /*INOUT*/ &dest,
  std::vector<int> const &maxima,
  int totalToDistribute,
  int /*INOUT*/ &nextColumnForUnevenDistribution);


#endif // SMQTUTIL_COL_WIDTH_RULES_H
