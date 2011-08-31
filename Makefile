CXX=g++
CXXFLAGS=-Wall -Werror -O3

OBJS=\
	utils/fatal.o\
	utils/time.o\
	utils/datafile.o\

TMPLS=\
	search/idastar.hpp\
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

include utils/Make.inc
include tiles/Make.inc

everything: $(BINS) $(TMPLS:.hpp=.gch)

%.gch: %.hpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -f $(OBJS) $(BINS) $(TMPLS:.hpp=.gch)