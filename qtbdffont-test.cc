// qtbdffont-test.cc
// Non-GUI tests for qtbdffont module.

#include "qtbdffont.h"                 // module to test

// this directory
#include "editor14r.bdf.gen.h"         // bdfFontData_editor14r

// smbase
#include "smbase/bdffont.h"            // BDFFont

using namespace smbase;


// Called from unit-tests.cc.
void test_qtbdffont()
{
  // This test is just that we can parse this one font without throwing
  // an exception or crashing.
  BDFFont font;
  parseBDFString(font, bdfFontData_editor14r);
}


// EOF
