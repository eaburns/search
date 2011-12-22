GRIDNAVOBJS=\
	gridnav/gridmap.o\
	gridnav/gridnav.o\
	gridnav/scenario.o\

include $(GRIDNAVOBJS:.o=.d)
CLEAN+=$(GRIDNAVOBJS)

gridnav: gridnav/scenario_solver gridnav/gridnav_solver gridnav/2poly

EVERYTHING+=gridnav

gridnav/scenario_solver: $(GRIDNAVOBJS) gridnav/scenario_main.o\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include gridnav/scenario_main.d
CLEAN+=gridnav/scenario_solver gridnav/scenario_main.o

gridnav/gridnav_solver: $(GRIDNAVOBJS) gridnav/gridnav_main.o\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include gridnav/gridnav_main.d
CLEAN+=gridnav/gridnav_solver gridnav/gridnav_main.o

gridnav/2poly: $(GRIDNAVOBJS) gridnav/2poly.cc visnav/polymap.o visnav/visgraph.o\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include gridnav/2poly.d
CLEAN+=gridnav/2poly gridnav/2poly.o