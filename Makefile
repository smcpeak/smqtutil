# Makefile for smqtutil

# main target
all: libsmqtutil.a qtbdffont


# directories of other software
SMBASE := ../smbase
LIBSMBASE := $(SMBASE)/libsmbase.a

# C++ compiler, etc.
CXX    := g++
AR     := ar
RANLIB := ranlib

# flags for the C and C++ compilers (and preprocessor)
CCFLAGS := -g -Wall -I$(QTDIR)/include -I$(SMBASE) -Wno-deprecated

# flags for the linker
#
# The "qt-mt" is the multithreaded version.  I'm not using threads,
# but it seems that later Qts only have the MT version available.
LDFLAGS := -g -Wall $(LIBSMBASE) -L$(QTDIR)/lib -lqt-mt


# patterns of files to delete in the 'clean' target; targets below
# add things to this using "+="
TOCLEAN =


# ---------------- pattern rules --------------------
# compile .cc to .o
TOCLEAN += *.o *.d
%.o : %.cc
	$(CXX) -c -o $@ $< $(CCFLAGS)
	@perl $(SMBASE)/depend.pl -o $@ $< $(CCFLAGS) > $*.d


# Qt meta-object compiler
.PRECIOUS: moc_%.cc
TOCLEAN += moc_*.cc
moc_%.cc: %.h
	moc -o $@ $^


# Qt designer translator
%.h %.cc: %.ui
	uic -o $*.h $*.ui
	uic -o $*.cc -i $*.h $*.ui


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


# --------------- qtbdffont test program ----------------
QTBDFFONT_OBJS := $(filter-out qtbdffont.o,$(OBJS))

TOCLEAN += qtbdffont
qtbdffont: qtbdffont.h qtbdffont.cc $(QTBDFFONT_OBJS) $(LIBSMBASE)
	$(CXX) -o $@ $(CCFLAGS) -DTEST_QTBDFFONT qtbdffont.cc $(QTBDFFONT_OBJS) $(LDFLAGS)


# --------------------- misc ------------------------
clean:
	rm -f $(TOCLEAN)

check:
	@echo "no useful 'make check' at this time"

# EOF
