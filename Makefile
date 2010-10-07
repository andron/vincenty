# -*- mode:makefile; tab-width:2 -*-

.SECONDEXPANSION:

.DEFAULT: all

# Linking is default cpp-style not c.
LINK.o = $(LINK.cpp)

# Link and compile
vpath %.o   src test example
vpath %.cpp src test example
%.o:%.cpp
	$(COMPILE.cpp) $< $(OUTPUT_OPTION)
%.so:
	$(LINK.cpp) -shared \
		-Wl,-soname=$@.$(INTERFACEVERSION) -o $@.$(LIBVERSION) $^
	/sbin/ldconfig -n ./
	/bin/ln -sf $@.$(firstword $(subst ., ,$(LIBVERSION))) $@
%:%.o
	$(LINK.cpp) $^ $(OUTPUT_OPTION)

CXXFLAGS := -pipe -fpic -g3 -MMD -W -Wall -Wextra -pedantic -Weffc++ -I./include
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

ifdef opt
CXXFLAGS += \
	$(strip -ftree-loop-linear -funsafe-math-optimizations \
	-ftree-vectorize -ftree-vectorizer-verbose=1 \
	-fprefetch-loop-arrays -funroll-loops -funswitch-loops)
endif

all: libvincenty.so

libvincenty.so: LIBVERSION := 1.0.0
libvincenty.so: INTERFACEVERSION := $(firstword $(subst ., ,$(LIBVERSION)))
libvincenty.so: vincenty.o vincenty_geotypes.o vincenty_ostream.o coordinate_grid.o

LDFLAGS_TEST := -lgtest_main -lvincenty

TARGET_TEST := test.vincenty test.coordinate_grid test.angle test.square
test: all $(TARGET_TEST)
$(TARGET_TEST): libvincenty.so
$(TARGET_TEST): LDFLAGS += $(LDFLAGS_TEST)

example: example.coordinate_grid
example.coordinate_grid: LDFLAGS := -Wl,--as-needed -Wl,-rpath=./ -L./
example.coordinate_grid: example.coordinate_grid.o libvincenty.so

check: test
	./test.vincenty
	./test.coordinate_grid

distclean:
	@echo $@...
	@-find -type f -name "*\.o" -delete
	@-rm -f  *.so* *.o *~ *.d \
	test.vincenty \
	test.coordinate_grid \
	test.angle \
	test.square \
	example.coordinate_grid

doc:
	doxygen

.PHONY: all test check distclean doc Makefile
