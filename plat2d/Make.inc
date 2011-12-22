PLAT2DOBJS=\
	plat2d/lvl.o\
	plat2d/tile.o\
	plat2d/body.o\
	plat2d/player.o\
	plat2d/plat2d.o\

include $(PLAT2DOBJS:.o=.d)
CLEAN+=$(PLAT2DOBJS)

plat2d: plat2d/plat2d_solver plat2d/draw plat2d/play plat2d/2poly

EVERYTHING+=plat2d

plat2d/plat2d_solver: $(PLAT2DOBJS) plat2d/main.o\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include plat2d/main.d
CLEAN+=plat2d/plat2d_solver plat2d/main.o

plat2d/draw: $(PLAT2DOBJS) plat2d/draw.o utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include plat2d/draw.d
CLEAN+=plat2d/draw plat2d/draw.o

plat2d/2poly: $(PLAT2DOBJS) plat2d/2poly.o visnav/visgraph.a utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

include plat2d/2poly.d
CLEAN+=plat2d/2poly plat2d/2poly.o

SDLVER=$(shell sdl-config --version | sed 's/\([0-9]\)\.\([0-9]\).*/\1\2/')

plat2d/play: $(PLAT2DOBJS) plat2d/play.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -DSDLVER=$(SDLVER) -lSDL -o $@

include plat2d/play.d
CLEAN+=plat2d/play plat2d/play.o