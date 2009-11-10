# -*- mode:makefile; tab-width:3 -*-

.SECONDEXPANSION:

.DEFAULT: all

all: libvincenty.so

-include *.d

CXXFLAGS := -pipe -fpic -g -MMD -W -Wall -Wextra -pedantic -Weffc++

ifdef tune
CXXFLAGS += -O3 -mtune=native
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

LIBVERSION := 1.0.0
libvincenty.so: LDFLAGS := -Wl,-no-undefined
libvincenty.so: INTERFACEVERSION := $(firstword $(subst ., ,$(LIBVERSION)))
libvincenty.so: vincenty.o vincenty_geotypes.o vincenty_ostream.o coordinate_grid.o
	$(LINK.cpp) -shared -Wl,-soname=$@.$(INTERFACEVERSION) -o $@.$(LIBVERSION) $^
	/sbin/ldconfig -n ./
	/bin/ln -sf $@.$(firstword $(subst ., ,$(LIBVERSION))) $@

test: libvincenty.so test.o
test: LDFLAGS := -Wl,--as-needed -Wl,-rpath=./ -L./ -lvincenty -lgtest_main

check: test
	./test

distclean:
	-rm -f test *.so* *.o *~ *.d

doc:
	doxygen
