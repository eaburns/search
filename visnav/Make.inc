VISNAVOBJS=\
	visnav/visgraph.o\
	visnav/visnav.o\

include $(VISNAVOBJS:.o=.d)
CLEAN+=$(VISNAVOBJS)

visnav/rand: $(VISNAVOBJS) visnav/rand.o utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include visnav/rand.d
CLEAN+=visnav/rand visnav/rand.o

visnav/draw: $(VISNAVOBJS) visnav/draw.o utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include visnav/draw.d
CLEAN+=visnav/draw visnav/draw.o

visnav/inst: $(VISNAVOBJS) visnav/inst.o utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include visnav/inst.d
CLEAN+=visnav/inst visnav/inst.o

visnav/visnav_solver: $(VISNAVOBJS) visnav/main.o\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include visnav/main.d
CLEAN+=visnav/visnav_solver visnav/main.o

visnav: visnav/rand visnav/draw visnav/inst visnav/visnav_solver

EVERYTHING+=visnav