TILESSRC:=\
	tiles/tiles.cc\
	tiles/mdist.cc\
	tiles/pdb.cc\

tiles: tiles/15md_solver tiles/15pdbgen

EVERYTHING+=tiles

DEFS15=-DWIDTH=4 -DHEIGHT=4

tiles/15md_solver: $(TILESSRC) tiles/main_mdist.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(DEFS15) -o $@

tiles/15pdbgen: $(TILESSRC) tiles/pdbgen.cc utils/utils.a 
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(DEFS15) -o $@

CLEAN+=$(TILESSRC:.cc=.o) tiles/15md_solver tiles/15pdbgen