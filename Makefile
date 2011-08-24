CXX=g++
CXXFLAGS=-Wall -Werror -O3

OBJS=\
	utils/fatal.o\

HDRS=\
	incl/utils.hpp\
	incl/search.hpp\
	\
	search/idastar.hpp\

BINS=

all: bins

include utils/Make.inc
include tiles/Make.inc

bins: $(OBJS) $(BINS)

clean:
	rm -f $(OBJS) $(BINS)