gridnav:\
	gridnav/scenario_solver\
	gridnav/gridnav_solver\
	gridnav/2poly\
	gridnav/draw\

GRIDNAVOBJS=\
	gridnav/gridmap.o\
	gridnav/gridnav.o\
	gridnav/scenario.o\

include $(GRIDNAVOBJS:.o=.d)
CLEAN+=$(GRIDNAVOBJS)

EVERYTHING+=gridnav

gridnav/scenario_solver:\
	$(GRIDNAVOBJS)\
	gridnav/main_scenario.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include gridnav/main_scenario.d
CLEAN+=gridnav/scenario_solver gridnav/main_scenario.o

gridnav/gridnav_solver:\
	$(GRIDNAVOBJS)\
	gridnav/main_gridnav.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $^ $(LDFLAGS) -o $@

include gridnav/main_gridnav.d
CLEAN+=gridnav/gridnav_solver gridnav/main_gridnav.o

gridnav/2poly:\
	$(GRIDNAVOBJS)\
	gridnav/2poly.cc\
	visnav/visgraph.a\
	graphics/image.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include gridnav/2poly.d
CLEAN+=gridnav/2poly gridnav/2poly.o

gridnav/draw:\
	$(GRIDNAVOBJS)\
	gridnav/draw.cc\
	graphics/image.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include gridnav/draw.d
CLEAN+=gridnav/draw gridnav/draw.o