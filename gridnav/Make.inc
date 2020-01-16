gridnav:\
	gridnav/scenario_solver\
	gridnav/gridnav_solver\
	gridnav/2poly\
	gridnav/draw\
	gridnav/mkseedinst\
	gridnav/scenario2rdb\
	gridnav/randinst\
	gridnav/ppm2grid\
	gridnav/watch\

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
	@$(CXX) $^ $(LDFLAGS) -o $@

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

gridnav/mkseedinst:\
	$(GRIDNAVOBJS)\
	gridnav/mkseedinst.cc\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include gridnav/mkseedinst.d
CLEAN+=gridnav/mkseedinst gridnav/mkseedinst.o

gridnav/scenario2rdb:\
	$(GRIDNAVOBJS)\
	gridnav/scenario2rdb.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a\
	rdb/rdb.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include gridnav/scenario2rdb.d
CLEAN+=gridnav/scenario2rdb gridnav/scenario2rdb.o

gridnav/randinst:\
	$(GRIDNAVOBJS)\
	gridnav/randinst.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a\
	rdb/rdb.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include gridnav/randinst.d
CLEAN+=gridnav/randinst gridnav/randinst.o

gridnav/ppm2grid:\
	gridnav/ppm2grid.cc\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include gridnav/ppm2grid.d
CLEAN+=gridnav/ppm2grid gridnav/ppm2grid.o

gridnav/watch:\
	$(GRIDNAVOBJS)\
	gridnav/watch.cc\
	graphics/graphics.a\
	search/search.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) $(GRAPHICSLD) -o $@

include gridnav/watch.d
CLEAN+=gridnav/watch gridnav/watch.o