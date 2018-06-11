# Makefile for smqtutil

# main target
all: libsmqtutil.a qtbdffont


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
%.bdf.gen.cc: fonts/%.bdf
	perl $(SMBASE)/file-to-strlit.pl bdfFontData_$* $^ $*.bdf.gen.h $@

# This is needed in case 'make' decides it needs the header file.
# I don't use the multi-target syntax because that is broken in
# the case of parallel make.
%.bdf.gen.h: %.bdf.gen.cc
	@echo "dummy rule to make $@ from $^"

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
OBJS += qtbdffont.o
OBJS += qtutil.o
-include $(OBJS:.o=.d)


TOCLEAN += libsmqtutil.a
libsmqtutil.a: $(OBJS)
	rm -f $@
	$(AR) -r $@ $(OBJS)
	-$(RANLIB) $@


# ------------------- test-qtutil -----------------------
TOCLEAN += test-qtutil
test-qtutil: test-qtutil.cc qtutil.o
	$(CXX) -o $@ $(CCFLAGS) test-qtutil.cc qtutil.o $(LDFLAGS)


# --------------- qtbdffont test program ----------------
QTBDFFONT_OBJS := $(filter-out qtbdffont.o,$(OBJS))

TOCLEAN += qtbdffont
qtbdffont: qtbdffont.h qtbdffont.cc $(QTBDFFONT_OBJS) $(LIBSMBASE)
	$(CXX) -o $@ $(CCFLAGS) -DTEST_QTBDFFONT qtbdffont.cc $(QTBDFFONT_OBJS) $(LDFLAGS)


# --------------------- misc ------------------------
clean:
	$(RM) $(TOCLEAN)

check: test-qtutil
	./test-qtutil
	@echo "smqtutil tests PASSED"

# EOF
