# -*- mode:makefile; tab-width:3 -*-

.SECONDEXPANSION:

.DEFAULT: all

# Linking is default cpp-style not c.
LINK.o = $(LINK.cpp)

# Link and compile
%.so:
	$(LINK.cpp) -shared \
		-Wl,--version-script,link.ld \
		-Wl,-soname=$@.$(INTERFACEVERSION) -o $@.$(LIBVERSION) $^
	/sbin/ldconfig -n ./
	/bin/ln -sf $@.$(firstword $(subst ., ,$(LIBVERSION))) $@
%.o:%.cpp
	$(COMPILE.cpp) $< $(OUTPUT_OPTION)
%:%.o
	$(LINK.cpp) $^ $(OUTPUT_OPTION)

all: libvincenty.so

-include *.d

CXXFLAGS := -pipe -fpic -g3 -MMD -W -Wall -Wextra -pedantic -Weffc++
LDFLAGS := -Wl,--as-needed -Wl,--no-undefined -Wl,-rpath=./ -L./

ifdef tune
CXXFLAGS += -O3 -march=native
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
	$(strip -ftree-loop-linear -funsafe-math-optimizations \
	-ftree-vectorize -ftree-vectorizer-verbose=1 \
	-fprefetch-loop-arrays -funroll-loops -funswitch-loops)
endif

libvincenty.so: LDFLAGS := -Wl,--no-undefined
libvincenty.so: LIBVERSION := 1.0.0
libvincenty.so: INTERFACEVERSION := $(firstword $(subst ., ,$(LIBVERSION)))
libvincenty.so: vincenty.o vincenty_geotypes.o vincenty_ostream.o coordinate_grid.o

test: test_vincenty test_coordinate_grid test_angle_correction test_square_computation

test_vincenty: test_vincenty.o libvincenty.so
test_vincenty: LDFLAGS += -lgtest_main

test_coordinate_grid: test_coordinate_grid.o libvincenty.so
test_coordinate_grid: LDFLAGS += -lgtest_main

test_angle_correction: test_angle_correction.o libvincenty.so
test_angle_correction: LDFLAGS += -lgtest_main

test_square_computation: test_square_computation.o libvincenty.so
test_square_computation: LDFLAGS += -lgtest_main

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
	test_square_computation \
	example_coordinate_grid

doc:
	doxygen
