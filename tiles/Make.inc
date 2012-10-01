tiles:\
	tiles/15md_solver\
	tiles/24md_solver\
	tiles/generator\

CLEAN+=\
	tiles/15md_solver\
	tiles/24md_solver\
	tiles/generator\
	
EVERYTHING+=tiles

TILESSRC:=\
	tiles/tiles.cc\
	tiles/mdist.cc\
	tiles/pdb.cc\

CLEAN+=$(TILESSRC:.cc=.o)

tiles/%md_solver: tiles/main_mdist.d
tiles/%md_solver:\
	$(TILESSRC)\
	tiles/main_mdist.cc\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $(TILESSRC)\
		tiles/main_mdist.cc\
		utils/utils.a\
		structs/structs.a\
		search/search.a\
		$(LDFLAGS) `./tiles/defs.sh $*` -o $@

tiles/generator: tiles/generator.d
tiles/generator: tiles/generator.cc utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS)\
		tiles/generator.cc\
		utils/utils.a\
		$(LDFLAGS) -o $@

CLEAN+=tiles/generator

include tiles/main_mdist.d
