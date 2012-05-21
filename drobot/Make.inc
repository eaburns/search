DROBOTOBJ=\
	drobot/drobot.o\

include $(DROBOTOBJ:.o=.d)
CLEAN+=$(DROBOTOBJ)

drobot: drobot/drobot_solver

TESTS+=.drobot_test

.drobot_test: drobot/test
	./drobot/test ".*test" && touch $@
CLEAN+=.drobot_test

drobot/test: drobot/test.cc drobot/drobot.o utils/utils.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

EVERYTHING+=drobot

drobot/drobot_solver: $(DROBOTOBJ) drobot/main.o\
	utils/utils.a structs/structs.a search/search.a
	@echo $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include drobot/main.d
CLEAN+=drobot/drobot_solver drobot/main.o
