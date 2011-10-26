BINS+=\
	visnav/rand\

VISNAVSRC=\
	visnav/visgraph.cc\

VISNAVHDR=\
	visnav/visgraph.hpp\

visnav/*.o: $(VISNAVHDR)

visnav/rand: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/rand.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/rand.cc -o visnav/rand