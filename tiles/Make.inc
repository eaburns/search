BINS+=\
	tiles/15unitmd_solver\
	tiles/15mdlearn_solver\

TILESSRC=\
	tiles/tiles.cc\
	tiles/mdist.cc\
	tiles/mdlearn.cc\

TILESHDR=\
	tiles/tiles.hpp\
	tiles/packed.hpp\
	tiles/mdist.hpp\
	tiles/mdlearn.hpp\

tiles/15mdlearn_solver: $(OBJS) $(HDRS) $(TILESHDR) $(TILENSRC) tiles/main_mdlearn.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(TILESSRC)\
		-DWIDTH=4 -DHEIGHT=4 tiles/main_mdlearn.cc\
		-o tiles/15mdlearn_solver

tiles/15unitmd_solver: $(OBJS) $(HDRS) $(TILESHDR) $(TILENSRC) tiles/main_mdist.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(TILESSRC)\
		-DWIDTH=4 -DHEIGHT=4 tiles/main_mdist.cc\
		-o tiles/15unitmd_solver