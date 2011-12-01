VISNAVOBJS=\
	visnav/visgraph.o\
	visnav/visnav.o\

include $(VISNAVOBJS:.o=.d)

visnav: visnav/rand visnav/draw visnav/inst visnav/visnav_solver

EVERYTHING+=visnav

visnav/rand: $(VISNAVOBJS) visnav/rand.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

visnav/draw: $(VISNAVOBJS) visnav/draw.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

visnav/inst: $(VISNAVOBJS) visnav/inst.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

visnav/visnav_solver: $(VISNAVOBJS) visnav/main.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

CLEAN+=$(VISNAVOBJS)\
	visnav/rand\
	visnav/draw\
	visnav/inst\
	visnav/visnav_solver\