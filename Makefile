.PHONY: clean all
.SECONDARY:

top:=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

all:: libblake3d.a b3dsum unit

lib-src:=nothing.cc
b3dsum-src:=b3dsum.cc
unit-src:=unit.cc

all-src:=$(lib-src) $(b3dsum-src) $(unit-src)
all-obj:=$(patsubst %.cc, %.o, $(all-src))

gtest-top:=$(top)test/googletest/googletest
gtest-inc:=$(gtest-top)/include
gtest-src:=$(gtest-top)/src/gtest-all.cc

vpath %.cc $(top)lib
vpath %.cc $(top)b3dsum
vpath %.cc $(top)test

OPTFLAGS?=-O3 -march=native
#OPTFLAGS?=-O0 -fsanitize=address
CXXFLAGS+=$(OPTFLAGS) -MMD -MP -std=c++17 -g -pthread
CPPFLAGS+=-isystem $(gtest-inc) -I $(top)include

depends:=$(patsubst %.cc, %.d, $(all-src)) gtest.d
-include $(depends)

clean:
	rm -f $(all-obj)

realclean: clean
	rm -f unit b3dsum libblake3d.a gtest.o $(depends)


# library

lib-obj:=$(patsubst %.cc, %.o, $(lib-src))
libblake3d.a: $(lib-obj)
	$(AR) rcs $@ $^


# command-line tool

b3dsum-obj:=$(patsubst %.cc, %.o, $(b3dsum-src))
b3dsum: CPPFLAGS+=-I$(top)b3dsum -isystem $(top)b3dsum/tinyopt/include
b3dsum: $(b3dsum-obj) libblake3d.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)


# unit tests

gtest.o: CPPFLAGS+=-I $(gtest-top)
gtest.o: ${gtest-src}
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

unit-obj:=$(patsubst %.cc, %.o, $(unit-src))
unit: $(unit-obj) gtest.o libblake3d.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

