unknowngridnav:\
	unknowngridnav/scenario_solver\
	unknowngridnav/gridnav_solver\
	unknowngridnav/watch\

UNKNOWNGRIDNAVOBJS=\
	unknowngridnav/unknowngridmap.o\
	unknowngridnav/unknowngridnav.o\

include $(UNKNOWNGRIDNAVOBJS:.o=.d)
CLEAN+=$(UNKNOWNGRIDNAVOBJS)

EVERYTHING+=unknowngridnav

unknowngridnav/scenario_solver:\
	$(UNKNOWNGRIDNAVOBJS)\
	unknowngridnav/main_unknownscenario.o\
	gridnav/scenario.o\
	gridnav/gridnav.o\
	gridnav/gridmap.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include unknowngridnav/main_unknownscenario.d
CLEAN+=unknowngridnav/scenario_solver unknowngridnav/main_scenario.o

unknowngridnav/gridnav_solver:\
	$(UNKNOWNGRIDNAVOBJS)\
	unknowngridnav/main_unknowngridnav.o\
	gridnav/gridnav.o\
	gridnav/gridmap.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $^ $(LDFLAGS) -o $@

include unknowngridnav/main_unknowngridnav.d
CLEAN+=unknowngridnav/unknowngridnav_solver unknowngridnav/main_unknowngridnav.o

unknowngridnav/watch:\
	$(UNKNOWNGRIDNAVOBJS)\
	unknowngridnav/watch.cc\
	gridnav/gridnav.o\
	gridnav/gridmap.o\
	visnav/visgraph.a\
	graphics/graphics.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) $(GRAPHICSLD) -o $@

include unknowngridnav/watch.d
CLEAN+=unknowngridnav/watch unknowngridnav/watch.o