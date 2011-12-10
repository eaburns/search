PANCAKESRC=\
	pancake/pancake.cc\

pancake: pancake/50pancake_solver pancake/70pancake_solver

EVERYTHING+=pancake

pancake/50pancake_solver: $(PANCAKESRC) pancake/main.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -DNCAKES=50 -o $@

pancake/70pancake_solver: $(PANCAKESRC) pancake/main.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -DNCAKES=70  -o $@

CLEAN+=$(PANCAKESRC:.cc=.o) pancake/50pancake_solver pancake/70pancake_solver