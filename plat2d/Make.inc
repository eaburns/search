PLAT2DOBJS=\
	plat2d/lvl.o\
	plat2d/tile.o\
	plat2d/geom.o\
	plat2d/player.o\
	plat2d/plat2d.o\

include $(PLAT2DOBJS:.o=.d)

plat2d: plat2d/plat2d_solver plat2d/draw plat2d/play

EVERYTHING+=plat2d

plat2d/plat2d_solver: $(PLAT2DOBJS) plat2d/main.cc\
	utils/utils.a structs/structs.a search/search.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

plat2d/draw: $(PLAT2DOBJS) plat2d/draw.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

SDLVER=$(shell sdl-config --version | sed 's/\([0-9]\)\.\([0-9]\).*/\1\2/')

plat2d/play: $(PLAT2DOBJS) plat2d/play.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -DSDLVER=$(SDLVER) -lSDL -o $@

CLEAN+=$(PLAT2DOBJS) plat2d/plat2d_solver plat2d/draw plat2d/play