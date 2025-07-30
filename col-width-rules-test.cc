// col-width-rules-test.cc
// Tests for `col-width-rules`.

#include "smbase/gdvalue-optional-fwd.h"         // gdv::toGDValue(std::optional)
#include "smbase/gdvalue-vector-fwd.h"           // gdv::toGDValue(std::vector)

#include "col-width-rules.h"                     // this module

#include "smbase/exc.h"                          // EXN_CONTEXT[_EXPR]
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/gdvalue-optional.h"             // gdv::toGDValue(std::optional)
#include "smbase/gdvalue-vector.h"               // gdv::toGDValue(std::vector)
#include "smbase/ordered-map-ops.h"              // GDVOrderedMap (for TEST_CASE_EXPRS)
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-span.h"                      // smbase::Span
#include "smbase/sm-test.h"                      // DIAG, EXPECT_EQ, TEST_CASE_EXPRS

#include <iostream>                              // std::cout (h)
#include <optional>                              // std::optional

using namespace gdv;


OPEN_ANONYMOUS_NAMESPACE


void oneTest_flexibility(
  int minSize,
  std::optional<int> maxSize,
  int curSize,
  int maxFlex,
  int expectContract,
  int expectExpand)
{
  EXN_CONTEXT("flexiblity");
  EXN_CONTEXT_EXPR(minSize);
  EXN_CONTEXT_EXPR(toGDValue(maxSize));
  EXN_CONTEXT_EXPR(curSize);
  EXN_CONTEXT_EXPR(maxFlex);

  ColumnWidthRules::ColSpec cs(minSize, maxSize, 1 /*prio*/);

  EXPECT_EQ(cs.flexibility(curSize, false /*expand*/, maxFlex), expectContract);
  EXPECT_EQ(cs.flexibility(curSize, true /*expand*/, maxFlex), expectExpand);
}


void test_flexibility()
{
  // Most flexible, start in middle.
  oneTest_flexibility(0, {}, 10, 99, 10, 99);

  // Start at left.
  oneTest_flexibility(0, {}, 0, 99, 0, 99);

  // Start at a large value.
  oneTest_flexibility(0, {}, 200, 99, 200, 99);

  // Non-zero minimum.
  oneTest_flexibility(10, {}, 10, 99, 0, 99);
  oneTest_flexibility(10, {}, 15, 99, 5, 99);
  oneTest_flexibility(10, {}, 5, 99, 0, 99);

  // A different `maxFlex`.
  oneTest_flexibility(10, {}, 20, 1000, 10, 1000);

  // Specified maximum.
  oneTest_flexibility(0, 100, 10, 99, 10, 90);
  oneTest_flexibility(0, 100, 0, 99, 0, 100);
  oneTest_flexibility(0, 100, 100, 99, 100, 0);

  // Both specified.
  oneTest_flexibility(30, 100, 10, 99, 0, 90);
  oneTest_flexibility(30, 100, 30, 99, 0, 70);
  oneTest_flexibility(30, 100, 50, 99, 20, 50);
  oneTest_flexibility(30, 100, 100, 99, 70, 0);
  oneTest_flexibility(30, 100, 110, 99, 80, 0);
}


// Verify one `resizeAll` call.
void oneTest_resizeAll(
  ColumnWidthRules &rules,
  std::vector<int> const &initSizes,
  int newTotalSize,
  std::vector<int> const &expectSizes)
{
  TEST_CASE_EXPRS("resizeAll", rules, initSizes, newTotalSize);

  std::vector<int> actualSizes(initSizes);

  bool changed = rules.resizeAll(actualSizes, newTotalSize);

  xassert(changed == (initSizes != actualSizes));

  EXPECT_EQ(toGDValue(actualSizes), toGDValue(expectSizes));
}


// Verify one `resizeSome` call.
void oneTest_resizeSome(
  ColumnWidthRules &rules,
  int startColumnIndex,
  std::vector<int> const &initSizes,
  int newTotalSize,
  std::vector<int> const &expectSizes)
{
  TEST_CASE_EXPRS("resizeSome",
    startColumnIndex, rules, initSizes, newTotalSize);

  std::vector<int> actualSizes(initSizes);

  bool changed =
    rules.resizeSome(startColumnIndex, actualSizes, newTotalSize);

  xassert(changed == (initSizes != actualSizes));

  EXPECT_EQ(toGDValue(actualSizes), toGDValue(expectSizes));
}


// This also tests `resizeSome`.
void test_resizeAll()
{
  // ChatGPT helped write some of these tests.

  {
    // Simple initial case: all have a minimum, none have a max, and one
    // column normally expands/contracts.
    ColumnWidthRules rules({
      { 30, {}, 1 },
      { 30, {}, 0 },
      { 30, {}, 0 },
    });

    EXPECT_EQ(rules.numColumns(), 3);

    // Simple expansion case.
    oneTest_resizeAll(rules,
      { 200, 100, 100 },
      500,
      { 300, 100, 100 });

    // And contraction.
    oneTest_resizeAll(rules,
      { 200, 100, 100 },
      300,
      { 100, 100, 100 });

    // No change needed.
    oneTest_resizeAll(rules,
      { 200, 100, 100 },
      400,
      { 200, 100, 100 });

    // Cannot shrink since all are already at minimum.
    oneTest_resizeAll(rules,
      { 30, 30, 30 },
      10,
      { 30, 30, 30 });

    oneTest_resizeSome(rules, 1,
      { 30, 30 },
      100,
      { 50, 50 });

    oneTest_resizeSome(rules, 2,
      { 30 },
      100,
      { 100 });

    // Exercise `clear`.
    rules.clear();
    EXPECT_EQ(rules.numColumns(), 0);
  }

  {
    // Larger minimum for the first column.
    ColumnWidthRules rules({
      { 150, {}, 1 },
      { 30, {}, 0 },
      { 30, {}, 0 },
    });

    // Try to shrink below min.
    oneTest_resizeAll(rules,
      { 200, 100, 100 },
      200,
      { 150, 30, 30 });

    // Negative total -> minima.
    oneTest_resizeAll(rules,
      { 200, 100, 100 },
      -1000,
      { 150, 30, 30 });
  }

  // Some ChatGPT-generated tests explicitly name the type.
  typedef ColumnWidthRules::ColSpec ColSpec;

  {
    // First has a max.
    ColumnWidthRules rules({
      ColSpec(30, 120, 1),
      ColSpec(30, {}, 0),
      ColSpec(30, {}, 0),
    });

    // First expands only up to its max.
    oneTest_resizeAll(rules,
      { 100, 100, 100 },
      400,
      { 120, 140, 140 });
  }


  // Multiple priorities.
  {
    ColumnWidthRules rules({
      ColSpec(30, {}, 2),
      ColSpec(30, {}, 1),
      ColSpec(30, {}, 0),
    });

    // Highest priority takes all expansion.
    oneTest_resizeAll(rules,
      { 100, 100, 100 },
      400,
      { 200, 100, 100 });
  }

  // Multiple priorities + max limit.
  {
    ColumnWidthRules rules({
      ColSpec(30, 150, 2),
      ColSpec(30, {}, 1),
      ColSpec(30, {}, 0),
    });

    // Highest priority hits max, lower priority fills rest.
    oneTest_resizeAll(rules,
      { 100, 100, 100 },
      400,
      { 150, 150, 100 });

    // Highest priority shrinks first.
    oneTest_resizeAll(rules,
      { 100, 100, 100 },
      180,
      { 30, 50, 100 });

    // Test with negative `newTotalSize`.  Effect is to set all to their
    // minima.
    oneTest_resizeAll(rules,
      { 100, 100, 100 },
      -1,
      { 30, 30, 30 });

    oneTest_resizeSome(rules, 1,
      { 100, 100 },
      300,
      { 200, 100 });
  }

  // Expansion with multiple columns of same priority.
  {
    ColumnWidthRules rules({
      ColSpec(30, {}, 1),
      ColSpec(30, {}, 1),
      ColSpec(30, {}, 1),
    });

    // Even distribution at same priority.
    oneTest_resizeAll(rules,
      { 100, 100, 100 },
      400,
      { 134, 133, 133 });
  }
}


// Verify one `resizeOne` call.
void oneTest_resizeOne(
  ColumnWidthRules &rules,
  int focusColumn,
  std::vector<int> const &initSizes,
  int desiredTotalSize,
  std::vector<int> const &expectSizes)
{
  TEST_CASE_EXPRS("resizeOne",
    focusColumn, rules, initSizes, desiredTotalSize);

  std::vector<int> actualSizes(initSizes);

  bool changed =
    rules.resizeOne(focusColumn, actualSizes, desiredTotalSize);

  xassert(changed == (initSizes != actualSizes));

  EXPECT_EQ(toGDValue(actualSizes), toGDValue(expectSizes));
}


void test_resizeOne()
{
  // Normally, resizing the middle column causes the rightmost to adjust
  // to take remaining space.
  {
    ColumnWidthRules rules({
      { 30, {} },
      { 30, {} },
      { 30, {} },
    });

    // This input corresponds to the user trying to change the size of
    // the middle column to 100.
    oneTest_resizeOne(rules, 1,
      { 100, 100, 30 },
      300,
      { 100, 100, 100 });
  }

  // But if the rightmost has a maximum size, we need to expand the
  // middle column as well.
  {
    ColumnWidthRules rules({
      { 30, {} },
      { 30, {} },
      { 30, 50 },
    });

    oneTest_resizeOne(rules, 1,
      { 100, 100, 30 },
      300,
      { 100, 150, 50 });
  }

  // And if the middle has a max as well, then left expands.
  {
    ColumnWidthRules rules({
      { 30, {} },
      { 30, 50 },
      { 30, 50 },
    });

    oneTest_resizeOne(rules, 1,
      { 100, 100, 30 },
      300,
      { 200, 50, 50 });
  }

  // And if the left has a max too, then we max them all out, even
  // though the size goal is unsatisfied.
  {
    ColumnWidthRules rules({
      { 30, 50 },
      { 30, 50 },
      { 30, 50 },
    });

    oneTest_resizeOne(rules, 1,
      { 30, 100, 30 },
      300,
      { 50, 50, 50 });
  }
}


void oneTest_evenlyDistribute(
  std::vector<int> const &maxima,
  int totalToDistribute,
  std::vector<int> const &expect,
  int inputNextUneven,
  int expectOutputNextUneven)
{
  TEST_CASE_EXPRS("evenlyDistribute",
    maxima, totalToDistribute, inputNextUneven);

  std::vector<int> actual(maxima.size(), 0);
  int nextUneven = inputNextUneven;
  evenlyDistribute(actual, maxima, totalToDistribute,
    nextUneven /*INOUT*/);

  EXPECT_EQ(toGDValue(actual), toGDValue(expect));
  EXPECT_EQ(nextUneven, expectOutputNextUneven);
}


void test_evenlyDistribute()
{
  // Test cases originally written by ChatGPT, but subsequently
  // modified:

  // Basic equal spread
  oneTest_evenlyDistribute({5, 5, 5}, 6, {2, 2, 2}, 0, 0);

  // Respect maxima
  oneTest_evenlyDistribute({1, 5, 5}, 6, {1, 3, 2}, 0, 2);

  // More to give than maxima allows
  oneTest_evenlyDistribute({1, 1, 1}, 10, {1, 1, 1}, 0, 0);

  // Exact fit
  oneTest_evenlyDistribute({2, 2, 2}, 6, {2, 2, 2}, 0, 0);

  // Partial last round
  oneTest_evenlyDistribute({2, 2, 2}, 5, {2, 2, 1}, 0, 2);

  // Zero total
  oneTest_evenlyDistribute({2, 2, 2}, 0, {0, 0, 0}, 0, 0);

  // One slot
  oneTest_evenlyDistribute({5}, 3, {3}, 0, 0);

  // Uneven maxima
  oneTest_evenlyDistribute({1, 2, 3}, 5, {1, 2, 2}, 0, 0);

  // More by me:

  // No slots.
  oneTest_evenlyDistribute({}, 3, {}, 0, 0);

  // All slots are full.
  oneTest_evenlyDistribute({0, 0, 0}, 3, {0, 0, 0}, 0, 0);

  // One slot is not full, but gets filled, and there is still more.
  oneTest_evenlyDistribute({10, 0, 0}, 20, {10, 0, 0}, 0, 0);

  // Focus on the `nextUneven` behavior:

  // Series of one-element distributions.
  oneTest_evenlyDistribute({10, 10, 10}, 1, {1, 0, 0}, 0, 1);
  oneTest_evenlyDistribute({ 9, 10, 10}, 1, {0, 1, 0}, 1, 2);
  oneTest_evenlyDistribute({ 9,  9, 10}, 1, {0, 0, 1}, 2, 0);
  oneTest_evenlyDistribute({ 9,  9,  9}, 1, {1, 0, 0}, 0, 1);

  // Two-element distributions.
  oneTest_evenlyDistribute({10, 10, 10}, 2, {1, 1, 0}, 0, 2);
  oneTest_evenlyDistribute({ 9,  9, 10}, 2, {1, 0, 1}, 2, 1);
  oneTest_evenlyDistribute({ 8,  9,  9}, 2, {0, 1, 1}, 1, 0);
  oneTest_evenlyDistribute({ 8,  8,  8}, 2, {1, 1, 0}, 0, 2);

  // One-element distributions over different maxima to eventually fill
  // all of the space.  The final two steps do not change `next` since
  // its loop does not activate.
  oneTest_evenlyDistribute({2,  3,  1}, 1, {1, 0, 0}, 0, 1);
  oneTest_evenlyDistribute({1,  3,  1}, 1, {0, 1, 0}, 1, 2);
  oneTest_evenlyDistribute({1,  2,  1}, 1, {0, 0, 1}, 2, 0);
  oneTest_evenlyDistribute({1,  2,  0}, 1, {1, 0, 0}, 0, 1);
  oneTest_evenlyDistribute({0,  2,  0}, 1, {0, 1, 0}, 1, 1);
  oneTest_evenlyDistribute({0,  1,  0}, 1, {0, 1, 0}, 1, 1);

  // Clamp the incoming value.
  oneTest_evenlyDistribute({5,  5,  5}, 1, {0, 1, 0}, 7, 2);


  // Big numbers.
  int million = 1000000;
  oneTest_evenlyDistribute(
    {million, million, million},
    4*million,
    {million, million, million},
    0, 0);
}


CLOSE_ANONYMOUS_NAMESPACE


// Called from unit-tests.cc.
void test_col_width_rules()
{
  test_evenlyDistribute();
  test_flexibility();
  test_resizeAll();
  test_resizeOne();
}


// EOF
