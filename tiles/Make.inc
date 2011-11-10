BINS+=\
	tiles/15md_solver\

TILESSRC=\
	tiles/tiles.cc\
	tiles/mdist.cc\
	tiles/pdb.cc\

TILESHDR=\
	tiles/tiles.hpp\
	tiles/packed.hpp\
	tiles/mdist.hpp\
	tiles/pdb.hpp\

tiles/*.o: $(TILESHDR)

tiles/15md_solver: $(OBJS) $(HDRS) $(TILESHDR) $(TILESSRC) tiles/main_mdist.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(TILESSRC)\
		-DWIDTH=4 -DHEIGHT=4 tiles/main_mdist.cc\
		-o tiles/15md_solver