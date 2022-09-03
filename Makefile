# Makefile for smqtutil

# main target
all: libsmqtutil.a qtutil-test test-qtbdffont test-layout


# ------------------- BEGIN: Configuration ---------------------
# This is where we set variables that depend on the build or
# target platform.  The rest of the Makefile should take care
# of responding to these variables without further intervention.

# Directories of other software.
SMBASE := ../smbase

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


# Flags for the C and C++ compilers (and preprocessor).
CCFLAGS := -g -Wall -Wno-deprecated -std=c++11
CCFLAGS += -I$(SMBASE)
CCFLAGS += $(QT_CCFLAGS)
CCFLAGS += $(EXTRA_CCFLAGS)

# Flags for the linker.
LDFLAGS := -g -Wall $(SMBASE)/libsmbase.a
LDFLAGS += $(QT_LDFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)


# patterns of files to delete in the 'clean' target; targets below
# add things to this using "+="
TOCLEAN = $(QT_TOCLEAN)


# ---------------- pattern rules --------------------
# Compile .cc to .o .
# -MMD causes GCC to write .d file.
# The -MP modifier adds phony targets to deal with removed headers.
TOCLEAN += *.o *.d
%.o : %.cc
	$(CXX) -c -MMD -MP -o $@ $< $(CCFLAGS)


# ---------------- default fonts --------------------
%.bdf.gen.cc %.bdf.gen.h: fonts/%.bdf
	perl $(SMBASE)/file-to-strlit.pl bdfFontData_$* $^ $*.bdf.gen.h $@

BDFGENSRC :=
BDFGENSRC += editor14b.bdf.gen.cc
BDFGENSRC += editor14i.bdf.gen.cc
BDFGENSRC += editor14r.bdf.gen.cc
BDFGENSRC += lurs12.bdf.gen.cc
BDFGENSRC += minihex6.bdf.gen.cc

.PHONY: gensrc
gensrc: $(BDFGENSRC)

TOCLEAN += $(BDFGENSRC)
TOCLEAN += $(BDFGENSRC:.cc=.h)


# ------------------- main library -------------------
OBJS :=
OBJS += $(BDFGENSRC:.cc=.o)
OBJS += qhboxframe.o
OBJS += qtbdffont.o
OBJS += qtguiutil.o
OBJS += qtutil.o
OBJS += sm-line-edit.o
OBJS += timer-event-loop.o
-include $(OBJS:.o=.d)


TOCLEAN += libsmqtutil.a
libsmqtutil.a: $(OBJS)
	$(RM) $@
	$(AR) -r $@ $(OBJS)
	-$(RANLIB) $@


# ------------------- qtutil-test -----------------------
TEST_PROGRAMS := qtutil-test
qtutil-test: qtutil-test.cc qtguiutil.o qtutil.o
	$(CXX) -o $@ $(CCFLAGS) $^ $(LDFLAGS)


# ------------------ test-qtbdffont ---------------------
TEST_PROGRAMS += test-qtbdffont
test-qtbdffont: test-qtbdffont.cc $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) test-qtbdffont.cc $(OBJS) $(LDFLAGS)


# -------------------- test-layout ----------------------
TEST_PROGRAMS += test-layout
test-layout: test-layout.cc $(OBJS)
	$(CXX) -o $@ $(CCFLAGS) test-layout.cc $(OBJS) $(LDFLAGS)


# ----------------------- misc --------------------------
clean:
	$(RM) $(TOCLEAN) $(TEST_PROGRAMS)

check: $(TEST_PROGRAMS)
	./qtutil-test
	./test-qtbdffont
	@echo "smqtutil tests PASSED"

# EOF
