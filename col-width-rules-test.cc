// col-width-rules-test.cc
// Tests for `col-width-rules`.

#include "smbase/gdvalue-optional-fwd.h"         // gdv::toGDValue(std::optional)
#include "smbase/gdvalue-vector-fwd.h"           // gdv::toGDValue(std::vector)

#include "col-width-rules.h"                     // this module

#include "smbase/exc.h"                          // EXN_CONTEXT[_EXPR]
#include "smbase/gdvalue.h"                      // gdv::GDValue
#include "smbase/gdvalue-optional.h"             // gdv::toGDValue(std::optional)
#include "smbase/gdvalue-vector.h"               // gdv::toGDValue(std::vector)
#include "smbase/ordered-map-ops.h"              // GDVOrderedMap
#include "smbase/sm-pp-util.h"                   // SM_PP_MAP
#include "smbase/sm-macros.h"                    // OPEN_ANONYMOUS_NAMESPACE
#include "smbase/sm-test.h"                      // USUAL_TEST_MAIN, DIAG, EXPECT_EQ

#include <iostream>                              // std::cout (h)
#include <optional>                              // std::optional

using namespace gdv;


/*
  Print `stuff` in verbose mode, and push it onto the exception context
  stack.

  When combined with the `gdvalue` module, it can be used like this:

    TEST_CASE("resizeAll: " << GDValue(GDVOrderedMap{
      GDV_SKV_EXPR(rules),
      GDV_SKV_EXPR(initSizes),
      GDV_SKV_EXPR(newTotalSize),
    }).asIndentedString());

  to nicely format several pieces of structured data.

  This is a candidate to move to sm-test.h.
*/
#define TEST_CASE(stuff) \
  DIAG(stuff);           \
  EXN_CONTEXT(stuff) /* user ; */


/*
  Print/context each of several argument expressions.

  Use it like:

    TEST_CASE_EXPRS("resizeAll", rules, initSizes, newTotalSize);

  which expands to what is shown in the example above.

  This is a candidate to move to sm-test.h.
*/
#define TEST_CASE_EXPRS(label, ...)                        \
  TEST_CASE(label ": " << gdv::GDValue(gdv::GDVOrderedMap{ \
    SM_PP_COMMA_MAP(GDV_SKV_EXPR, __VA_ARGS__)             \
  }).asIndentedString()) /* user ; */



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
  ColumnWidthRules const &rules,
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
  }

  // Expansion with multiple columns of same priority.
  {
    ColumnWidthRules rules({
      ColSpec(30, {}, 1),
      ColSpec(30, {}, 1),
      ColSpec(30, {}, 1),
    });

    oneTest_resizeAll(rules,
      { 100, 100, 100 },
      400,
      { 134, 133, 133 }); // Even distribution at same priority.
  }
}


void oneTest_evenlyDistribute(
  std::vector<int> const &maxima,
  int totalToDistribute,
  std::vector<int> const &expect)
{
  TEST_CASE_EXPRS("evenlyDistribute", maxima, totalToDistribute);

  std::vector<int> actual(maxima.size(), 0);
  evenlyDistribute(actual, maxima, totalToDistribute);

  EXPECT_EQ(toGDValue(actual), toGDValue(expect));
}


void test_evenlyDistribute()
{
  // Test cases written by ChatGPT:

  // Basic equal spread
  oneTest_evenlyDistribute({5, 5, 5}, 6, {2, 2, 2});

  // Respect maxima
  oneTest_evenlyDistribute({1, 5, 5}, 6, {1, 3, 2});

  // More to give than maxima allows
  oneTest_evenlyDistribute({1, 1, 1}, 10, {1, 1, 1});

  // Exact fit
  oneTest_evenlyDistribute({2, 2, 2}, 6, {2, 2, 2});

  // Partial last round
  oneTest_evenlyDistribute({2, 2, 2}, 5, {2, 2, 1});

  // Zero total
  oneTest_evenlyDistribute({2, 2, 2}, 0, {0, 0, 0});

  // One slot
  oneTest_evenlyDistribute({5}, 3, {3});

  // Uneven maxima
  oneTest_evenlyDistribute({1, 2, 3}, 5, {1, 2, 2});

  // More by me:

  // No slots.
  oneTest_evenlyDistribute({}, 3, {});

  // All slots are full.
  oneTest_evenlyDistribute({0, 0, 0}, 3, {0, 0, 0});

  // One slot is not full, but gets filled, and there is still more.
  oneTest_evenlyDistribute({10, 0, 0}, 20, {10, 0, 0});

  // Big numbers.
  int million = 1000000;
  oneTest_evenlyDistribute(
    {million, million, million},
    4*million,
    {million, million, million});
}


void entry()
{
  test_evenlyDistribute();
  test_flexibility();
  test_resizeAll();
}


CLOSE_ANONYMOUS_NAMESPACE


USUAL_TEST_MAIN


// EOF
