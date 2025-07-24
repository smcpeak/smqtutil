# Makefile for smqtutil

# Default target.  (Its prereqs are added below.)
all:


# ------------------- BEGIN: Configuration ---------------------
# This is where we set variables that depend on the build or
# target platform.  The rest of the Makefile should take care
# of responding to these variables without further intervention.

# Directory root from which dependent libraries are expected to be
# found.
DEPSDIR := ..

# smbase library specifically
SMBASE = $(DEPSDIR)/smbase

# Build tools.
CXX    := g++
AR     := ar
RANLIB := ranlib

# Additional compile/link flags.
EXTRA_CCFLAGS :=
EXTRA_LDFLAGS :=

# Pull in build configuration.  This must provide definitions of
# QT5INCLUDE, QT5LIB and QT5BIN.  It can optionally override the
# variables defined above.
ifeq (,$(wildcard config.mk))
$(error The file config.mk does not exist.  You have to copy config.mk.template to config.mk and then edit the latter by hand)
endif
include config.mk
# -------------------- END: Configuration ----------------------


# Set QT_CCFLAGS, QT_LDFLAGS, and define rule for running 'moc'.
include qtvars.mk

# C++ standard to use.  smbase now requires C++17, so this library does too.
CPPSTD = c++17

# Flags for the C and C++ compilers (and preprocessor).
CCFLAGS = -g -Wall -Wno-deprecated -std=$(CPPSTD)
CCFLAGS += -I$(DEPSDIR)
CCFLAGS += $(QT_CCFLAGS)
CCFLAGS += $(EXTRA_CCFLAGS)

# Flags for the linker.
LDFLAGS := -g -Wall $(SMBASE)/obj/libsmbase.a
LDFLAGS += $(QT_LDFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)


# patterns of files to delete in the 'clean' target; targets below
# add things to this using "+="
TOCLEAN = $(QT_TOCLEAN)


# Run whatever command follows with a timeout.
TIMEOUT_PROGRAM = timeout
TIMEOUT_VALUE = 10
RUN_WITH_TIMEOUT = $(TIMEOUT_PROGRAM) $(TIMEOUT_VALUE)


# ---------------- pattern rules --------------------
# Compile .cc to .o .
# -MMD causes GCC to write .d file.
# The -MP modifier adds phony targets to deal with removed headers.
%.o : %.cc
	$(CXX) -c -MMD -MP -o $@ $< $(CCFLAGS)


# ---------------- default fonts --------------------
%.bdf.gen.cc %.bdf.gen.h: fonts/%.bdf
	perl $(SMBASE)/file-to-strlit.pl bdfFontData_$* $^ $*.bdf.gen.h $@

BDFGENSRC :=
BDFGENSRC += courB24_ISO8859_1.bdf.gen.cc
BDFGENSRC += courO24_ISO8859_1.bdf.gen.cc
BDFGENSRC += courR24_ISO8859_1.bdf.gen.cc
BDFGENSRC += editor14b.bdf.gen.cc
BDFGENSRC += editor14i.bdf.gen.cc
BDFGENSRC += editor14r.bdf.gen.cc
BDFGENSRC += lurs12.bdf.gen.cc
BDFGENSRC += minihex6.bdf.gen.cc

.PHONY: gensrc
gensrc: $(BDFGENSRC)


# ------------------- main library -------------------
OBJS :=
OBJS += $(BDFGENSRC:.cc=.o)
OBJS += col-width-rules.o
OBJS += qhboxframe.o
OBJS += qtbdffont.o
OBJS += qtguiutil.o
OBJS += qtutil.o
OBJS += sm-line-edit.o
OBJS += sm-table-widget.moc.o
OBJS += sm-table-widget.o
OBJS += timer-event-loop.o
-include $(OBJS:.o=.d)


all: libsmqtutil.a
libsmqtutil.a: $(OBJS)
	$(RM) $@
	$(AR) -r $@ $(OBJS)
	-$(RANLIB) $@


# ------------------- qtutil-test -----------------------
TEST_PROGRAMS :=
TEST_PROGRAMS += qtutil-test.exe
qtutil-test.exe: qtutil-test.o qtutil-test.moc.o qtguiutil.o qtutil.o
	$(CXX) -o $@ $(CCFLAGS) $^ $(LDFLAGS)


# ------------------ test-qtbdffont ---------------------
TEST_PROGRAMS += test-qtbdffont.exe
test-qtbdffont.exe: test-qtbdffont.cc $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) test-qtbdffont.cc $(OBJS) $(LDFLAGS)


# -------------------- test-layout ----------------------
TEST_PROGRAMS += test-layout.exe
test-layout.exe: test-layout.cc $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) test-layout.cc $(OBJS) $(LDFLAGS)


# ------------------------ sm-table-widget-test ------------------------
TEST_PROGRAMS += sm-table-widget-test.exe
sm-table-widget-test.exe: sm-table-widget-test.cc $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) sm-table-widget-test.cc $(OBJS) $(LDFLAGS)


# ------------------------ col-width-rules-test ------------------------
TEST_PROGRAMS += col-width-rules-test.exe
col-width-rules-test.exe: col-width-rules-test.cc $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) col-width-rules-test.cc $(OBJS) $(LDFLAGS)


# ----------------------- misc --------------------------
all: $(TEST_PROGRAMS)

clean:
	$(RM) *.a *.d *.o *.exe *.gen.* $(TOCLEAN)

check: all
	$(RUN_WITH_TIMEOUT) ./qtutil-test.exe
	$(RUN_WITH_TIMEOUT) ./test-qtbdffont.exe
	$(RUN_WITH_TIMEOUT) ./col-width-rules-test.exe
	@echo "smqtutil tests PASSED"

# EOF
