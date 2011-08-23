CXX=g++
CXXFLAGS=-Wall -Werror -g\
	-DWIDTH=4 -DHEIGHT=4


OBJS=\
	utils/fatal.o\

HDRS=\
	incl/utils.hpp\

BINS=

all: bins

include utils/Make.inc
include tiles/Make.inc

bins: $(OBJS) $(BINS)

clean:
	rm -f $(OBJS) $(BINS)