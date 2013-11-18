# -*- mode:make; tab-width:2; -*-

include $(HEADER)

NAME     := vincenty
VERSION  := 1.0.0
RELEASE  := 1
REQUIRES := # Nothing
$(call setup)

_CXXFLAGS := -march=native

TARGETS := libvincenty.so

include $(FOOTER)
