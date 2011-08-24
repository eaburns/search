BINS:=\
	15unitmd_solver\

TILESRC=\
	tiles/tiles.cc\

15unitmd_solver: $(OBJS) $(HDRS) tiles/tiles.cc tiles/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(TILESRC) tiles/main.cc\
		-DWIDTH=4 -DHEIGHT=4 -o 15unitmd_solver