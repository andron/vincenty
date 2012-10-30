# -*- mode:make; tab-width:2; -*-

include $(HEADER)

NAME     := vincenty
VERSION  := 1.0.0
RELEASE  := 1
REQUIRES :=
$(call setup_requires)

_CXXFLAGS := -march=native

TARGETS := libvincenty.so

include $(FOOTER)
