CXX=g++
CXXFLAGS=-Wall -Werror -O3

OBJS=\
	utils/fatal.o\
	utils/time.o\
	utils/datafile.o\
	utils/testing.o\

TMPLS=\
	search/idastar.hpp\
	search/openlist.hpp\
	\
	structs/intpq.hpp\
	structs/binheap.hpp\
	structs/htable.hpp\

HDRS=\
	incl/utils.hpp\
	incl/search.hpp\

HDRS+=$(TMPLS)

BINS=

all: everything

include structs/Make.inc
include utils/Make.inc
include tiles/Make.inc

everything: $(TMPLS:.hpp=.hpp.gch) $(BINS)

%.hpp.gch: %.hpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -f $(OBJS) $(BINS) $(TMPLS:.hpp=.hpp.gch)