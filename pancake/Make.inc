pancake:\
	pancake/50pancake_solver\
	pancake/70pancake_solver

CLEAN+=\
	pancake/50pancake_solver\
	pancake/70pancake_solver

EVERYTHING+=pancake

PANCAKESRC=\
	pancake/pancake.cc\

CLEAN+=$(PANCAKESRC:.cc=.o)

pancake/%pancake_solver: pancake/main.d
pancake/%pancake_solver:\
	$(PANCAKESRC)\
	pancake/main.cc\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) $(PANCAKESRC)\
		pancake/main.cc\
		utils/utils.a\
		structs/structs.a\
		search/search.a\
		$(LDFLAGS) -DNCAKES=$*  -o $@

include pancake/main.d
