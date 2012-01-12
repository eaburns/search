TILESSRC:=\
	tiles/tiles.cc\
	tiles/mdist.cc\
	tiles/pdb.cc\

CLEAN+=$(TILESSRC:.cc=.o)

tiles: tiles/15md_solver tiles/15pdbgen

EVERYTHING+=tiles

DEFS15=-DWIDTH=4 -DHEIGHT=4

tiles/15md_solver: tiles/main_mdist.d
tiles/15md_solver: $(TILESSRC) tiles/main_mdist.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $(TILESSRC) tiles/main_mdist.cc utils/utils.a structs/structs.a search/search.a $(LDFLAGS) $(DEFS15) -o $@

CLEAN+=tiles/15md_solver
include tiles/main_mdist.d