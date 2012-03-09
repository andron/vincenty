# -*- mode:make; tab-width:2; -*-

include $(HEADER)

TARGETS := test.reg.vincenty test.reg.coordinategrid

# These apply to all targets in this makerules.
_CXXFLAGS := -fipa-pta
_LDFLAGS := -pthread
_LINK := vincenty gtest_main gtest

test.reg.vincenty_SRCS := test.vincenty.cpp
test.reg.coordinategrid_SRCS := test.coordinate_grid.cpp

include $(FOOTER)
