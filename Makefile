CXX=g++
FLAGS=-Wall -Werror -O3
CXXFLAGS=$(FLAGS)
CFLAGS=$(FLAGS)

OBJS=\
	utils/fatal.o\
	utils/time.o\
	utils/datafile.o\
	utils/testing.o\
	utils/hash.o\
	utils/rand.o\
	utils/encode.o\
	utils/image.o\
	\
	structs/stn.o\
	\
	search/search.o\

TMPLS=\
	search/main.hpp\
	search/openlist.hpp\
	search/idastar.hpp\
	search/astar.hpp\
	\
	structs/intpq.hpp\
	structs/binheap.hpp\
	structs/htable.hpp\
	structs/stn.hpp\

HDRS=\
	utils/utils.hpp\
	utils/image.hpp\
	\
	search/search.hpp\

HDRS+=$(TMPLS)

BINS=

all: everything

*/*.o: $(HDRS)

include structs/Make.inc
include utils/Make.inc
include tiles/Make.inc
include pancake/Make.inc
include gridpath/Make.inc

everything: $(TMPLS:.hpp=.hpp.gch) $(BINS)

%.hpp.gch: %.hpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -f $(OBJS) $(BINS) $(TMPLS:.hpp=.hpp.gch)
