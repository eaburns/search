BINS:=\
	15unitmd_solver\

15unitmd_solver: $(OBJS) tiles/tiles.cc tiles/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) tiles/tiles.cc tiles/main.cc\
		-o 15unitmd_solver