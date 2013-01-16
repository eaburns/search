plat2d:\
	plat2d/plat2d_solver\
	plat2d/makeh\
	plat2d/draw\
	plat2d/2poly\
	plat2d/watch\

PLAT2DOBJS=\
	plat2d/lvl.o\
	plat2d/tile.o\
	plat2d/body.o\
	plat2d/player.o\

include $(PLAT2DOBJS:.o=.d)
CLEAN+=$(PLAT2DOBJS)

EVERYTHING+=plat2d

plat2d/plat2d_solver:\
	$(PLAT2DOBJS)\
	plat2d/main.o\
	plat2d/plat2d.o\
	graphics/image.o\
	visnav/visgraph.a\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $^ $(LDFLAGS) -o $@

include plat2d/main.d plat2d/plat2d.d
CLEAN+=plat2d/plat2d_solver plat2d/main.o plat2d/plat2d.o

plat2d/makeh:\
	$(PLAT2DOBJS)\
	plat2d/makeh.o\
	plat2d/plat2d.o\
	graphics/image.o\
	visnav/visgraph.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include plat2d/makeh.d
CLEAN+=plat2d/makeh plat2d/makeh.o

plat2d/draw:\
	$(PLAT2DOBJS)\
	plat2d/draw.o\
	graphics/image.o\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include plat2d/draw.d
CLEAN+=plat2d/draw plat2d/draw.o

plat2d/2poly:\
	$(PLAT2DOBJS)\
	plat2d/2poly.o\
	graphics/image.o\
	visnav/visgraph.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include plat2d/2poly.d
CLEAN+=plat2d/2poly plat2d/2poly.o

plat2d/watch:\
	$(PLAT2DOBJS)\
	plat2d/watch.cc\
	plat2d/plat2d.o\
	visnav/visgraph.a\
	graphics/graphics.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) $(GRAPHICSLD) -o $@

include plat2d/watch.d
CLEAN+=plat2d/watch plat2d/watch.o