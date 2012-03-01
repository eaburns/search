#CXX:=clang++ -fno-color-diagnostics
#CC:=clang -fno-color-diagnostics

CXX:=g++ -std=c++98
CC:=gcc -std=c99

AR:=ar

FLAGS:=-Wall -Werror -O3

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
include graphics/Make.inc
include bench/Make.inc
include rdb/Make.inc

everything: $(EVERYTHING)

tests: $(TESTS)

%.d: %.cc
	./dep.sh g++ $(shell dirname $*) $(CXXFLAGS) $*.cc > $@

%.d: %.c
	./dep.sh gcc $(shell dirname $*) $(CFLAGS) $*.c > $@

clean:
	rm -f $(CLEAN) $(BINS) $(TMPLS:.hpp=.hpp.gch)

nuke: clean
	rm -f $(shell find . -name \*.d) 
