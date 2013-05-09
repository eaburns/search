traffic:\
	traffic/traffic_solver traffic/watch\
	traffic/randinst\

TRAFFICOBJS=\
	traffic/gridmap.o\
	traffic/traffic.o\

include $(TRAFFICOBJS:.o=.d)
CLEAN+=$(TRAFFICOBJS)

EVERYTHING+=traffic

traffic/traffic_solver:\
	$(TRAFFICOBJS)\
	traffic/main_traffic.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $^ $(LDFLAGS) -o $@

include traffic/main_traffic.d
CLEAN+=traffic/traffic_solver traffic/main_traffic.o

traffic/watch:\
	$(TRAFFICOBJS)\
	traffic/watch.cc\
	traffic/traffic.o\
	visnav/visgraph.a\
	graphics/graphics.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) $(GRAPHICSLD) -o $@

include traffic/watch.d
CLEAN+=traffic/watch traffic/watch.o

traffic/randinst:\
	$(TRAFFICOBJS)\
	traffic/randinst.cc\
	utils/utils.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include traffic/watch.d
CLEAN+=traffic/randinst traffic/randinst.o
