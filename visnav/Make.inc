BINS+=\
	visnav/visnav_solver\

VISNAVSRC=\
	visnav/poly.cc\

VISNAVHDR=\
	visnav/poly.hpp\

visnav/*.o: $(VISNAVHDR)

visnav/visnav_solver: $(OBJS) $(HDRS) $(VISNAVHDR) $(VISNAVSRC) visnav/main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(VISNAVSRC)\
		visnav/main.cc -o visnav/visnav_solver