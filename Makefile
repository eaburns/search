CXX:=g++

CC:=gcc

AR:=ar

FLAGS:=-Wall -Werror -O0 -g

CXXFLAGS:=$(FLAGS)

CFLAGS:=$(FLAGS)

LDFLAGS:=

EVERYTHING:=

TESTS:=

all: everything tests

include utils/Make.inc
include structs/Make.inc
include search/Make.inc
include tiles/Make.inc
include pancake/Make.inc
include gridnav/Make.inc
include visnav/Make.inc
include plat2d/Make.inc

everything: $(EVERYTHING)

tests: $(TESTS)

%.d: %.cc
	./dep.sh $(CXX) $(shell dirname $*) $(CXXFLAGS) $*.cc > $@

%.d: %.c
	./dep.sh $(CC) $(shell dirname $*) $(CFLAGS) $*.c > $@

clean:
	rm -f $(CLEAN) $(BINS) $(TMPLS:.hpp=.hpp.gch)

nuke: clean
	rm -f $(shell find . -name \*.d) 