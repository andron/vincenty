# -*- mode:make; tab-width:2; -*-

include $(HEADER)

NAME     := vincenty
VERSION  := 1.0.0
RELEASE  := 1
REQUIRES :=
$(call setup_requires)

TARGETS := libvincenty.so

_CXXFLAGS := -march=native

libvincenty.so_SRCS := $(call getsource,src,*.cpp)

include $(FOOTER)
