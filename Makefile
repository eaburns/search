CXX:=g++
CC:=gcc

#
# clang 3.0 should work.
# clang 2.9 is known to be broken, as it can't seem to
#	compile the GNU header files…
#
#CXX:=clang++ -fno-color-diagnostics
#CC:=clang -fno-color-diagnostics

AR:=ar

FLAGS:=-Wall -Werror -g -O3

CXXFLAGS:=$(FLAGS) -std=c++0x

CFLAGS:=$(FLAGS) -std=c99

LDFLAGS:=$(FLAGS) -std=c++0x -lrt

EVERYTHING:=

TESTS:=

all: everything tests

include utils/Make.inc
include rdb/Make.inc
include structs/Make.inc
include search/Make.inc
include tiles/Make.inc
include pancake/Make.inc
include gridnav/Make.inc
include visnav/Make.inc
include plat2d/Make.inc
include graphics/Make.inc
include drobot/Make.inc
include segments/Make.inc
include vacuum/Make.inc
include traffic/Make.inc

everything: $(EVERYTHING)

tests: $(TESTS)

%.o: %.c
	@echo $@
	@$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cc
	@echo $@
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

%.d: %.cc
	@echo $@
	@./dep.sh $(CXX) $(shell dirname $*) $(CXXFLAGS) $*.cc > $@

%.d: %.c
	@echo $@
	@./dep.sh $(CC) $(shell dirname $*) $(CFLAGS) $*.c > $@

clean:
	rm -f $(CLEAN) $(BINS) $(TMPLS:.hpp=.hpp.gch)

nuke: clean
	rm -f $(shell find . -not -iwholename \*.hg\* -name \*.d)
