BINS+=\
	visnav/rand\
	visnav/disp\
	visnav/prob\
	visnav/visnav_solver\

VISNAVSRC=\
	visnav/visgraph.cc\
	visnav/visnav.cc\

VISNAVHDR=\
	visnav/visgraph.hpp\
	visnav/visnav.hpp\

visnav/*.o: $(VISNAVHDR)

visnav/rand: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/rand.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/rand.cc -o visnav/rand

visnav/disp: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/disp.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/disp.cc -o visnav/disp

visnav/prob: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/prob.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/prob.cc -o visnav/prob

visnav/visnav_solver: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/main.cc -o visnav/visnav_solver