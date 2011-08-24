BINS:=\
	tiles/15unitmd_solver\

TILESSRC=\
	tiles/tiles.cc\

tiles/15unitmd_solver: $(OBJS) $(HDRS) $(TILENSRC) tiles/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(TILESSRC)\
		-DWIDTH=4 -DHEIGHT=4 tiles/main.cc\
		-o tiles/15unitmd_solver