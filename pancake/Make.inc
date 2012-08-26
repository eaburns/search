pancake: pancake/50pancake_solver pancake/70pancake_solver

PANCAKESRC=\
	pancake/pancake.cc\

CLEAN+=$(PANCAKESRC:.cc=.o)

include pancake/main.d

pancake/50pancake_solver: pancake/main.d
pancake/50pancake_solver:\
	$(PANCAKESRC)\
	pancake/main.cc\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $(PANCAKESRC) pancake/main.cc utils/utils.a structs/structs.a search/search.a $(LDFLAGS) -DNCAKES=50 -o $@

CLEAN+=pancake/15pancake_solver

pancake/70pancake_solver: pancake/main.d
pancake/70pancake_solver:\
	$(PANCAKESRC)\
	pancake/main.cc\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $(PANCAKESRC) pancake/main.cc utils/utils.a structs/structs.a search/search.a $(LDFLAGS) -DNCAKES=70  -o $@

CLEAN+=pancake/70pancake_solver

EVERYTHING+=pancake