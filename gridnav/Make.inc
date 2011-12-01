GRIDNAVOBJS=\
	gridnav/gridmap.o\
	gridnav/gridnav.o\
	gridnav/scenario.o\

include $(GRIDNAVOBJS:.o=.d)

gridnav: gridnav/scenario_solver gridnav/gridnav_solver gridnav/2poly

EVERYTHING+=gridnav

gridnav/scenario_solver: $(GRIDNAVOBJS) gridnav/scenario_main.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

gridnav/gridnav_solver: $(GRIDNAVOBJS) gridnav/gridnav_main.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

gridnav/2poly: $(GRIDNAVOBJS) gridnav/2poly.cc visnav/visgraph.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

CLEAN+=$(GRIDNAVOBJS)\
	gridnav/scenario_solver\
	gridnav/gridnav_solver\
	gridnav/2poly