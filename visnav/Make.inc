visnav:\
	visnav/visgraph.a\
	visnav/visnav_solver\
	visnav/rand\
	visnav/draw\
	visnav/inst\
	visnav/mkgraph\
	visnav/pbm2poly\

VISNAVOBJS=\
	visnav/polymap.o\
	visnav/grid2poly.o\
	visnav/visgraph.o\
	visnav/visnav.o\

include $(VISNAVOBJS:.o=.d)
CLEAN+=$(VISNAVOBJS)

visnav/rand:\
	$(VISNAVOBJS)\
	visnav/rand.o\
	graphics/image.o\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include visnav/rand.d
CLEAN+=visnav/rand visnav/rand.o

visnav/draw:\
	$(VISNAVOBJS)\
	visnav/draw.o\
	graphics/image.o\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include visnav/draw.d
CLEAN+=visnav/draw visnav/draw.o

visnav/inst:\
	$(VISNAVOBJS)\
	visnav/inst.o\
	graphics/image.o\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include visnav/inst.d
CLEAN+=visnav/inst visnav/inst.o

visnav/visnav_solver:\
	$(VISNAVOBJS)\
	visnav/main.o\
	graphics/image.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $^ $(LDFLAGS) -o $@

include visnav/main.d
CLEAN+=visnav/visnav_solver visnav/main.o

visnav/mkgraph:\
	$(VISNAVOBJS)\
	visnav/mkgraph.o\
	graphics/image.o\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include visnav/mkgraph.d
CLEAN+=visnav/mkgraph visnav/mkgraph.o

visnav/pbm2poly:\
	$(VISNAVOBJS)\
	visnav/pbm2poly.o\
	graphics/image.o\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include visnav/pbm2poly.d
CLEAN+=visnav/pbm2poly visnav/pbm2poly.o

visnav/visgraph.a:\
	visnav/polymap.o\
	visnav/grid2poly.o\
	visnav/visgraph.o
	@echo $@
	@$(AR) rsc $@ $^

CLEAN+=visnav/visgraph.a

EVERYTHING+=visnav