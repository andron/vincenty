# -*- mode:makefile; tab-width:3 -*-

.SECONDEXPANSION:

.DEFAULT: all

# Linking is default cpp-style not c.
LINK.o = $(LINK.cpp)

# Link and compile
%.so:
	$(LINK.cpp) -shared -Wl,-soname=$@.$(INTERFACEVERSION) -o $@.$(LIBVERSION) $^
	/sbin/ldconfig -n ./
	/bin/ln -sf $@.$(firstword $(subst ., ,$(LIBVERSION))) $@
%.o:%.cpp
	$(COMPILE.cpp) $< $(OUTPUT_OPTION)
%:%.o
	$(LINK.cpp) $^ $(OUTPUT_OPTION)

all: libvincenty.so

-include *.d

CXXFLAGS := -pipe -fpic -g -MMD -W -Wall -Wextra -pedantic -Weffc++
LDFLAGS := -Wl,--as-needed -Wl,--no-undefined -Wl,-rpath=./ -L./

ifdef tune
CXXFLAGS += -O3 -march=native -msse3
else
ifdef debug
CXXFLAGS += -O0
else
CXXFLAGS += -O2 -mtune=generic
endif
endif

ifdef unopt
CXXFLAGS += -mno-sse2 -mno-sse3
endif

ifdef opt
CXXFLAGS += \
	$(strip -ftree-loop-linear \
	-ftree-vectorize -ftree-vectorizer-verbose=1 \
	-fprefetch-loop-arrays -funroll-loops -funswitch-loops)
endif

libvincenty.so: LDFLAGS := -Wl,--no-undefined
libvincenty.so: LIBVERSION := 1.0.0
libvincenty.so: INTERFACEVERSION := $(firstword $(subst ., ,$(LIBVERSION)))
libvincenty.so: vincenty.o vincenty_geotypes.o vincenty_ostream.o coordinate_grid.o

test: test_vincenty test_coordinate_grid test_angle_correction
test_vincenty: test_vincenty.o libvincenty.so
test_vincenty: LDFLAGS += -lgtest_main

test_coordinate_grid: test_coordinate_grid.o libvincenty.so
test_coordinate_grid: LDFLAGS += -lgtest_main

test_angle_correction: test_angle_correction.o libvincenty.so
test_angle_correction: LDFLAGS +=

example: example_coordinate_grid
example_coordinate_grid: LDFLAGS := -Wl,--as-needed -Wl,-rpath=./ -L./
example_coordinate_grid: example_coordinate_grid.o libvincenty.so

check: test
	./test_vincenty
	./test_coordinate_grid

distclean:
	-rm -f  *.so* *.o *~ *.d \
	test_vincenty \
	test_coordinate_grid \
	test_angle_correction \
	example_coordinate_grid

doc:
	doxygen
