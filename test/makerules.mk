# -*- mode:make; tab-width:2; -*-

include $(HEADER)

TARGETS := test.reg.vincenty test.reg.coordinategrid

# These apply to all targets in this makerules.
_LDFLAGS := -pthread -Wl,-rpath=$(TGTDIR)
_LINK := vincenty gtest_main gtest

GTEST_SRCS := gtest-all.cc

test.reg.vincenty_SRCS := $(GTEST_SRCS) test.vincenty.cpp
test.reg.coordinategrid_SRCS := $(GTEST_SRCS) test.coordinate_grid.cpp

include $(FOOTER)
