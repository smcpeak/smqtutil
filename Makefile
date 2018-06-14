# Makefile for smqtutil

# main target
all: libsmqtutil.a test-qtutil test-qtbdffont test-layout


# directories of other software
SMBASE := ../smbase
LIBSMBASE := $(SMBASE)/libsmbase.a

# Directory with Qt headers, libraries, and programs.
QTDIR := /d/bld/qt5/qtbase

# Same, as a path that can be passed to the compiler.
QTDIR_CPLR := $(shell cygpath -m '$(QTDIR)')

# C++ compiler, etc.
CXX    := g++
AR     := ar
RANLIB := ranlib

# flags for the C and C++ compilers (and preprocessor)
CCFLAGS := -g -Wall -Wno-deprecated -std=c++11
CCFLAGS += -I$(SMBASE)
CCFLAGS += -I$(QTDIR_CPLR)/include
CCFLAGS += -I$(QTDIR_CPLR)/include/QtCore
CCFLAGS += -I$(QTDIR_CPLR)/include/QtGui
CCFLAGS += -I$(QTDIR_CPLR)/include/QtWidgets

# flags for the linker
LDFLAGS := -g -Wall $(LIBSMBASE)
LDFLAGS += -L$(QTDIR_CPLR)/lib -lQt5Widgets -lQt5Gui -lQt5Core

# Qt build tools
MOC := $(QTDIR)/bin/moc
UIC := $(QTDIR)/bin/uic


# patterns of files to delete in the 'clean' target; targets below
# add things to this using "+="
TOCLEAN =


# ---------------- pattern rules --------------------
# Compile .cc to .o .
# -MMD causes GCC to write .d file.
# The -MP modifier adds phony targets to deal with removed headers.
TOCLEAN += *.o *.d
%.o : %.cc
	$(CXX) -c -MMD -MP -o $@ $< $(CCFLAGS)


# Qt meta-object compiler
.PRECIOUS: moc_%.cc
TOCLEAN += moc_*.cc
moc_%.cc: %.h
	$(MOC) -o $@ $^


# Qt designer translator
%.h %.cc: %.ui
	$(UIC) -o $*.h $*.ui
	$(UIC) -o $*.cc -i $*.h $*.ui


# ---------------- default fonts --------------------
%.bdf.gen.cc %.bdf.gen.h: fonts/%.bdf
	perl $(SMBASE)/file-to-strlit.pl bdfFontData_$* $^ $*.bdf.gen.h $@

BDFGENSRC :=
BDFGENSRC += editor14b.bdf.gen.cc
BDFGENSRC += editor14i.bdf.gen.cc
BDFGENSRC += editor14r.bdf.gen.cc
BDFGENSRC += lurs12.bdf.gen.cc

.PHONY: gensrc
gensrc: $(BDFGENSRC)

TOCLEAN += $(BDFGENSRC)
TOCLEAN += $(BDFGENSRC:.cc=.h)


# ------------------- main library -------------------
OBJS :=
OBJS += $(BDFGENSRC:.cc=.o)
OBJS += qhboxframe.o
OBJS += qtbdffont.o
OBJS += qtutil.o
-include $(OBJS:.o=.d)


TOCLEAN += libsmqtutil.a
libsmqtutil.a: $(OBJS)
	$(RM) $@
	$(AR) -r $@ $(OBJS)
	-$(RANLIB) $@


# ------------------- test-qtutil -----------------------
TEST_PROGRAMS := test-qtutil
test-qtutil: test-qtutil.cc qtutil.o
	$(CXX) -o $@ $(CCFLAGS) test-qtutil.cc qtutil.o $(LDFLAGS)


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
	./test-qtutil
	./test-qtbdffont
	@echo "smqtutil tests PASSED"

# EOF
