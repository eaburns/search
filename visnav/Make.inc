VISNAVSRC=\
	visnav/visgraph.cc\
	visnav/visnav.cc\

VISNAVHDR=\
	visnav/visgraph.hpp\
	visnav/visnav.hpp\

visnav/*.o: $(VISNAVHDR)

BINS+=\
	visnav/rand\
	visnav/draw\
	visnav/inst\
	visnav/visnav_solver\

visnav/rand: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/rand.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/rand.cc -o visnav/rand

visnav/draw: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/draw.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/draw.cc -o visnav/draw

visnav/inst: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/inst.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/inst.cc -o visnav/inst

visnav/visnav_solver: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/main.cc -o visnav/visnav_solver