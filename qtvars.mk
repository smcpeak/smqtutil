# qtvars.mk
# Makefile fragment to set Qt build configuration variables.

# This file is meant to be included from an outer Makefile.


# Validate Qt configuration variables.
ifeq (,$(wildcard $(QT5INCLUDE)/QtCore))
$(error The QT5INCLUDE directory, "$(QT5INCLUDE)", does not have a "QtCore" subdirectory)
endif

ifeq (,$(wildcard $(QT5LIB)/libQt5Core*))
$(error The QT5LIB directory, "$(QT5LIB)", does not have "libQt5Core*" in it)
endif

ifeq (,$(wildcard $(QT5BIN)/moc)$(wildcard $(QT5BIN)/moc.exe))
$(error The QT5BIN directory, "$(QT5BIN)", does not have "moc" or "moc.exe" in it)
endif


# Qt compile flags.
QT_CCFLAGS :=
QT_CCFLAGS += -isystem ${QT5INCLUDE}
QT_CCFLAGS += -isystem ${QT5INCLUDE}/QtCore
QT_CCFLAGS += -isystem ${QT5INCLUDE}/QtGui
QT_CCFLAGS += -isystem ${QT5INCLUDE}/QtWidgets

# Qt has a macro called 'foreach'.  This collides with a method on some
# of the containers in smbase (e.g., StringVoidDict, which is used by
# StringSet).  This suppresses 'foreach' while retaining Q_FOREACH, so
# the functionality is still available.
#
# It also suppresses 'emit', 'signals', etc., which all have Q_XXX
# counterparts.
QT_CCFLAGS += -DQT_NO_KEYWORDS

# Qt link flags.
#
# Note: -rpath does not work on Windows.  Instead, $(QT5BIN) must be on
# the PATH to run the program.
QT_LDFLAGS := -L$(QT5LIB) -lQt5Widgets -lQt5Gui -lQt5Core -Wl,-rpath=$(QT5LIB)

# Qt build tools.
QT_MOC := $(QT5BIN)/moc
QT_RCC := $(QT5BIN)/rcc


# Pattern to run the Qt meta-object compiler.
.PRECIOUS: %.moc.cc
QT_TOCLEAN := *.moc.cc
%.moc.cc: %.h
	$(QT_MOC) -o $@ $^

# Pattern to run the Qt resource compiler.  This generates a C++ file
# that just has to be compiled and linked with the executable.  It has
# a static constructor that registers its resources on startup.
.PRECIOUS: %.qrc.gen.cc
QT_TOCLEAN += *.qrc.gen.cc
%.qrc.gen.cc: %.qrc
	$(QT_RCC) -o $@ $^

# EOF
